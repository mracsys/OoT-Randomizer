from __future__ import annotations
from abc import ABC, abstractmethod
from base64 import b64decode, b64encode
from enum import Enum, IntEnum
from json import dumps
from typing import Any, Optional, Literal, overload, TYPE_CHECKING

from Rom import Rom
from MQ import align4, align8, align16, align_file
from SceneList import RecordType, SCENE_EXTERNAL_REFERENCES
from Cutscenes import CutsceneCommand, CutsceneCommandID, CutsceneCommandTextList, cutscene_constructor_from_id

if TYPE_CHECKING:
    from Scene import SceneCutsceneData, SceneTransitionActorList, ScenePathList, RoomActorList, RoomObjectList, CollisionBgCamInfoList, SceneHeader, RoomHeader

def segment_address_offset(segment_address: int) -> int:
    return segment_address & 0x00FFFFFF


def segment_address_segment(segment_address: int) -> int:
    return segment_address >> 0x18


def unsegment_address(segment_address: int) -> tuple[int, int]:
    return segment_address_segment(segment_address), segment_address_offset(segment_address)


def create_segment_address(segment: int, offset: int) -> int:
    return (segment << 0x18) + offset

class SceneCacheFileException(Exception):
    def __init__(self, filename: str, record_type: str, record_version: int, max_version: int) -> None:
        super().__init__(f'Cannot parse cached scene file record of type {record_type} in file {filename} with version {record_version}. Latest supported version: {max_version}')

# File type values correspond to segment number from the segment address table
# https://wiki.cloudmodding.com/oot/Addresses#Segment_Addresses
# -----------------------------------
# | # | Destination                 |
# -----------------------------------
# | 0 | Direct                      |
# | 1 | title related?              |
# | 2 | Currently loaded scene file |
# | 3 | Currently loaded room file  |
# | 4 | gameplay_keep               |
# | 5 | gameplay_field_keep /       |
# |   | gameplay_dangeon_keep       |
# | 6 | Current object              |
# | 7 | link_animetion              |
# -----------------------------------
class FileType(Enum):
    Direct = 0
    Title = 1
    Scene = 2
    Room = 3
    GlobalKeep = 4
    SceneKeep = 5
    Object = 6
    PlayerAnimation = 7


def pack_properties(cls: Any, schema: list[tuple[str, Any]]) -> dict[str, Any]:
    packed = {}
    for var, _ in schema:
        if var not in cls.__dict__.keys():
            raise Exception(f'Could not serialize {cls.__class__.__name__} class. {var} is not a valid property.')
        value = cls.__dict__[var]
        if isinstance(value, int) or isinstance(value, float) or isinstance(value, str) or isinstance(value, bool) or value is None:
            packed[var] = value
        elif isinstance(value, list):
            if len(value) == 0:
                packed[var] = []
            elif isinstance(value[0], int) or isinstance(value[0], float) or isinstance(value[0], str) or isinstance(value, bool):
                packed[var] = value.copy()
            else:
                packed[var] = []
                for v in value:
                    if isinstance(v, list):
                        # Only applies to RoomMeshDLEntries opa/xlu tuples
                        e = []
                        if v[0] is None:
                            e.append(None)
                        else:
                            e.append([var, v[0].file.segment, v[0].vanilla_offset, v[0].type.value])
                        if v[1] is None:
                            e.append(None)
                        else:
                            e.append([var, v[1].file.segment, v[1].vanilla_offset, v[1].type.value])
                        packed[var].append(e)
                    elif v is None:
                        packed[var].append(None)
                    elif isinstance(v, DataRecord):
                        packed[var] = [var, v.file.segment, v.vanilla_offset, v.type.value]
                    elif isinstance(v, CutsceneCommand):
                        command = pack_properties(v, v.data_record_schema)
                        command['id'] = int(v.id) # manually cast IntEnum to int to avoid another conditional in the pack/unpack functions
                        packed[var].append(command)
                    else:
                        packed[var].append(pack_properties(v, v.data_record_schema))
        elif isinstance(value, DataRecord):
            packed[var] = [var, value.file.segment, value.vanilla_offset, value.type.value]
        elif isinstance(value, CutsceneCommand):
            packed[var] = pack_properties(value, value.data_record_schema)
            packed[var]['id'] = int(value.id) # manually cast IntEnum to int to avoid another conditional in the pack/unpack functions
        else:
            packed[var] = pack_properties(value, value.data_record_schema)
    return packed


def unpack_properties(record: Any, schema: list[tuple[str, Any]], data_records: dict[str, Any], file: FileDataRelocator) -> Any:
    for var, subcls in schema:
        if var not in record.__dict__.keys():
            raise Exception(f'Cannot import cached {record.__class__.__name__}. Unknown property {var}')
        if var not in data_records.keys():
            continue
        value = data_records[var]
        # Important that the first condition remains first to catch any instances
        # where an optional property is set to None instead of a non-primitive type.
        if isinstance(value, int) or isinstance(value, float) or isinstance(value, str) or isinstance(value, bool) or value is None:
            record.__dict__[var] = value
        elif not issubclass(subcls, DataRecord) and not issubclass(subcls, CutsceneCommand) and isinstance(value, list):
            if not isinstance(value, list):
                raise Exception(f'Cannot import cached {record.__class__.__name__}. Expected list value for {var}. Got {var.__class__.__name__}')
            if len(value) == 0:
                record.__dict__[var] = []
            elif isinstance(value[0], int) or isinstance(value[0], float) or isinstance(value[0], str) or isinstance(value[0], bool):
                record.__dict__[var] = value.copy()
            else:
                record.__dict__[var] = []
                for v in value:
                    # if isinstance(v, list):
                    #     # Only applies to RoomMeshDLEntries opa/xlu tuples
                    #     e = []
                    #     if v[0] is None:
                    #         e.append(None)
                    #     else:
                    #         subrecord = subcls()
                    #         e.append(unpack_properties(subrecord, subrecord.data_record_schema, v[0], file))
                    #     if v[1] is None:
                    #         e.append(None)
                    #     else:
                    #         subrecord = subcls()
                    #         e.append(unpack_properties(subrecord, subrecord.data_record_schema, v[1], file))
                    #     record.__dict__[var].append(e)
                    if v is None:
                        record.__dict__[var].append(None)
                    else:
                        subrecord = subcls()
                        if isinstance(subrecord, int) or isinstance(subrecord, float) or isinstance(subrecord, str) or isinstance(subrecord, bool):
                            print(value)
                            print(f'Cannot import cached {record.__class__.__name__}. Expected data record for {var}. Got {subrecord.__class__.__name__}')
                            raise Exception(f'{subrecord.__class__.__name__} object has no attribute \'data_record_schema\'')
                        unpack_properties(subrecord, subrecord.data_record_schema, v, file)
                        record.__dict__[var].append(subrecord)
        elif issubclass(subcls, CutsceneCommand):
            # Cutscene commands are self-contained, making it possible to expand nested command lists in one pass
            if isinstance(value, list):
                command_list = []
                for command in value:
                    if 'sub_commands' in command.keys():
                        command_commands = []
                        # Text command lists can have one of three subtypes for subcommands.
                        # All other cutscene command subcommands use one consistent subtype.
                        # This prevents using the class schema directly as multiple types per key
                        # are not implemented. Use a lookup function keyed on subcommand id instead.
                        for subcmd_data in command['sub_commands']:
                            if 'id' not in subcmd_data.keys():
                                raise Exception(f'Could not determine cutscene subcommand type from schema during cache import. Missing "id" key.')
                            subcmdcls = cutscene_constructor_from_id(subcmd_data['id'])
                            subcmd = subcmdcls()
                            unpack_properties(subcmd, subcmd.data_record_schema, subcmd_data, file)
                            subcmd.id = CutsceneCommandID(subcmd_data['id'])
                            command_commands.append(subcmd)
                        command['sub_commands'] = command_commands
                    cmd_id = CutsceneCommandID(command['id']) # cast from int to CutsceneCommandID to convert back to IntEnum
                    cmdcls = cutscene_constructor_from_id(command['id'])
                    del command['id']
                    cmd = cmdcls(**command)
                    cmd.id = cmd_id
                    command_list.append(cmd)
                record.__dict__[var] = command_list
            else:
                if 'sub_commands' in value.keys():
                    value_commands = []
                    # Text command lists can have one of three subtypes for subcommands.
                    # All other cutscene command subcommands use one consistent subtype.
                    # This prevents using the class schema directly as multiple types per key
                    # are not implemented. Use a lookup function keyed on subcommand id instead.
                    for subcmd_data in value['sub_commands']:
                        if 'id' not in subcmd_data.keys():
                            raise Exception(f'Could not determine cutscene subcommand type from schema during cache import. Missing "id" key.')
                        subcmdcls = cutscene_constructor_from_id(subcmd_data['id'])
                        subcmd = subcmdcls()
                        unpack_properties(subcmd, subcmd.data_record_schema, subcmd_data, file)
                        subcmd.id = CutsceneCommandID(subcmd_data['id'])
                        value_commands.append(subcmd)
                    value['sub_commands'] = value_commands
                cmd_id = CutsceneCommandID(value['id']) # cast from int to CutsceneCommandID to convert back to IntEnum
                cmdcls = cutscene_constructor_from_id(value['id'])
                del value['id']
                record.__dict__[var] = subcls(**value)
                record.__dict__[var].id = cmd_id
        elif issubclass(subcls, DataRecord):
            # List of records
            if isinstance(value[0], list):
                record.__dict__[var] = [None for _ in range(len(value))]
                idx = 0
                for json_record in value:
                    if json_record is not None:
                        # Display list tuple check
                        if isinstance(json_record[0], list) or isinstance(json_record[1], list):
                            record.__dict__[var][idx] = [None for _ in range(len(json_record))]
                            subidx = 0
                            for tuple_record in json_record:
                                if tuple_record is not None:
                                    file.linked_json_records.append((
                                        var,
                                        record,
                                        tuple_record[1],
                                        tuple_record[2],
                                        RecordType(tuple_record[3]),
                                        idx,
                                        subidx,
                                    ))
                                subidx += 1
                        else:
                            file.linked_json_records.append((
                                var,
                                record,
                                json_record[1],
                                json_record[2],
                                RecordType(json_record[3]),
                                idx,
                                -1,
                            ))
                    idx += 1
            # Single record
            else:
                if len(value) < 4:
                    print('uh oh')
                file.linked_json_records.append((
                    var,
                    record,
                    value[1],
                    value[2],
                    RecordType(value[3]),
                    -1,
                    -1,
                ))
        else:
            record.__dict__[var] = subcls()
            unpack_properties(record.__dict__[var], record.__dict__[var].data_record_schema, value, file)


class DataRecord:
    version: int = 1
    # Class metadata used to interface with JSON cache
    # data_record - class property + foreign class constructor
    data_record_schema: list[tuple[str, Any]] = []

    def __init__(self, file: FileDataRelocator, offset: int, length: int, delay_parsing: bool = False, store_in_file: bool = True) -> None:
        assert offset >= 0

        self.file: FileDataRelocator = file
        if store_in_file:
            file.data_records.append(self)
        self.type: RecordType = RecordType.Unknown
        self.start: int = file.start
        self.offset: int = offset
        self.vanilla_offset: int = offset
        self.length: int = length
        self.delay_parsing: bool = delay_parsing
        self.store_in_file: bool = store_in_file
        self.align: int = 4

        if length > 0 and not delay_parsing:
            self.data: bytearray = self.file.rom.read_bytes(self.start + offset, length)
        else:
            self.data: bytearray = bytearray()

    def is_empty(self):
        for b in self.data:
            if b != 0:
                return False
        return True

    def refresh_rom_data(self):
        self.data = self.file.rom.read_bytes(self.start + self.offset, self.length)

    @staticmethod
    def decode(file: FileDataRelocator, type: RecordType, offset: int = 0, length: int = 0) -> DataRecord:
        existing_record = file.get_existing_record_by_offset(offset, type)
        if existing_record is not None:
            return existing_record
        return DataRecord(file, offset, length)

    def decode_late(self) -> None:
        pass

    def encode(self) -> bytearray:
        return self.data

    def merge(self, other_record: DataRecord) -> tuple[DataRecord, DataRecord, int]:
        if other_record.type != self.type:
            raise NotImplementedError(f'Attempted to merge record of type {other_record.type.value} with record of type {self.type.value}')
        if self.file is not other_record.file:
            raise Exception(f'Tried to merge {self.type.value} records in different files {self.file.name} and {other_record.file.name}')
        # Record creation handles the case for equal offsets but non-equal lengths
        if self.offset < other_record.offset:
            low_record = self
            high_record = other_record
        else:
            low_record = other_record
            high_record = self
        if low_record.offset + low_record.length < high_record.offset:
            raise Exception(f'Tried to merge non-overlapping {self.type.value} records at 0x{self.offset:0>8x}, length 0x{self.length:0>8x} and 0x{other_record.offset:0>8x}, length 0x{other_record.length:0>8x}')
        # Verify overlapping bytes are identical
        lower_overlap = high_record.offset - low_record.offset
        for i in range(lower_overlap, low_record.length):
            if low_record.data[i] != high_record.data[i - lower_overlap]:
                raise Exception(f'Tried to merge mismatching {self.type.value} records at 0x{self.offset:0>8x}, length 0x{self.length:0>8x} and 0x{other_record.offset:0>8x}, length 0x{other_record.length:0>8x}. Mismatch at 0x{low_record.offset + i:0>8x} (lower: 0x{low_record.data[i]:0>2x}, upper: 0x{high_record.data[i - lower_overlap]:0>2x})')
        low_record.length = high_record.offset + high_record.length - low_record.offset
        low_record.refresh_rom_data()
        low_record._merge_in_file(low_record.file, high_record, lower_overlap)
        low_record.file.data_records.remove(high_record)
        return low_record, high_record, lower_overlap

    def _merge_in_file(self, file: FileDataRelocator, other_record: DataRecord, record_offset: int):
        raise NotImplementedError(f'Attempted to merge record of type {other_record.type.value} with record of type {self.type.value}')

    def get_segment_address(self) -> int:
        return create_segment_address(int(self.file.type.value), self.offset)

    def get_segment_address_bytes(self) -> bytes:
        return self.get_segment_address().to_bytes(4, 'big')

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, DataRecord):
            return NotImplemented
        return self.type == other.type and self.offset == other.offset and self.length == other.length

    def __hash__(self) -> int:
        return hash((self.type, self.offset, self.length))

    def __str__(self) -> str:
        return dumps(self.to_json(), default=lambda x: x.to_json(), indent=2)

    def __repr__(self) -> str:
        return f'{self.type.value} @ 0x{self.offset:0>6x}, 0x{self.length:0>6x} bytes (id {hex(id(self))})'

    def to_json(self) -> dict[str, Any]:
        record = {
            'version': self.version,
            'type': self.type.value,
            'start_offset': self.offset,
            'length': self.length,
            'end_offset': self.offset + self.length,
            'vanilla_offset': self.vanilla_offset,
            'delay_parsing': self.delay_parsing,
            'store_in_file': self.store_in_file,
            'data': b64encode(self.data).decode('ascii'),
            'align': self.align,
            'data_records': {},
        }
        record['data_records'] = pack_properties(self, self.data_record_schema)
        return record

    @classmethod
    def from_json(cls, file: FileDataRelocator, cache: dict[str, Any]) -> DataRecord:
        if cache['version'] != cls.version:
            raise SceneCacheFileException(file.name, cache['type'], cache['version'], cls.version)
        record = cls(file, cache['start_offset'], cache['length'], True, cache['store_in_file'])
        record.vanilla_offset = cache['vanilla_offset']
        record.data = b64decode(cache['data'])
        record.align = cache['align']
        # Not set in constructor as data should be read from json, not the ROM.
        # Still important if this record is re-parsed for whatever reason.
        record.delay_parsing = cache['delay_parsing']
        unpack_properties(record, record.data_record_schema, cache['data_records'], file)
        return record


class FileDataRelocator(ABC):
    version: int = 1
    segment: int = 1 # not valid for actual OOT scene/room files

    def __init__(self, rom: Rom, name: str, start: int, end: int, type: FileType) -> None:
        self.rom: Rom = rom
        self.name: str = name
        self.start: int = start
        self.end: int = end
        self.rom_start: Optional[int] = start
        self.vanilla_start: int = start
        self.type: FileType = type
        self.parsed: bool = False

        self.data_records: list[DataRecord] = []
        # Tuple data: class property to assign record to, class instance, segment, vanilla offset
        self.linked_json_records: list[tuple[str, Any, int, int, RecordType]] = []

    # Prevent saving the Rom object to pickle files
    def __getstate__(self):
        state = super().__getstate__()
        del state['rom']
        return state

    def __setstate__(self, state):
        self.__dict__.update(state)
        # Updated after unpickling in Patches.py
        self.rom = None

    def parse(self) -> None:
        # Parse file header
        self.parse_file_header()

    def finalize(self) -> None:
        # Sort records by offset
        self.sort_records()

        # Parse records with unknown lengths now that neighboring records exist to bound them
        self.parse_late_records()

        # Merge records where the pointers don't necessarily point to the start of the data record
        self.merge_records()

        # Add unknown data record from last referenced data record to end of file
        self.add_unknown_record_at_file_end()

        # Check for overlapping records
        self.check_for_overlapping_records()

        # Add unknown data records between sorted records
        self.add_unknown_records()

        # Mark parsing as complete
        self.parsed = True

    # Extra parsing already captured by json import
    def finalize_from_cache(self) -> None:
        for property, record, segment, offset, record_type, list_index, tuple_index in self.linked_json_records:
            if list_index >= 0:
                if tuple_index >= 0:
                    record.__dict__[property][list_index][tuple_index] = self.get_existing_record_by_vanilla_offset_and_segment(offset, record_type, segment)
                else:
                    record.__dict__[property][list_index] = self.get_existing_record_by_vanilla_offset_and_segment(offset, record_type, segment)
            else:
                record.__dict__[property] = self.get_existing_record_by_vanilla_offset_and_segment(offset, record_type, segment)
        self.linked_json_records = []
        self.sort_records()
        self.parsed = True

    @abstractmethod
    def parse_file_header(self, alternate: Optional[int] = None) -> DataRecord:
        raise NotImplementedError(f'Cannot parse file header for unknown file {self.name}')

    @abstractmethod
    def get_offset(self, cursor: int) -> tuple[int, Optional[FileDataRelocator]]:
        raise NotImplementedError(f'Cannot calculate byte offset from segment address for unknown file {self.name}')

    def sort_records(self) -> None:
        self.data_records.sort(key=lambda x: x.offset)

    # Records must be sorted prior to using this method!!!!
    def next_data_record(self, record: DataRecord) -> DataRecord:
        record_index = self.data_records.index(record)
        # Rudimentary checks if records are unsorted
        if record_index < len(self.data_records) - 1:
            record_index += 1 # next record if we're not at the end of the file
        if self.data_records[record_index].offset < record.offset:
            raise Exception(f'Attempted to find next data record in file without sorting records by offset!')
        return self.data_records[record_index]

    def get_record_length_from_neighbor(self, record: DataRecord) -> int:
        neighbor = self.next_data_record(record)
        if neighbor is record:
            length = self.end - self.start - record.offset
        else:
            length = neighbor.offset - record.offset
        return length

    def parse_late_records(self) -> None:
        late_records = [record for record in self.data_records if record.delay_parsing]
        while len(late_records) > 0:
            # Start from beginning of the list/file as direct neighbors
            # are more likely to be missing later in the file at first.
            # Don't waste time with pop() since the list is recreated for
            # potentially new records.
            record = late_records[0]
            assert record.type != RecordType.Unknown
            record.decode_late()
            if record.length == 0:
                raise Exception(f'Something went wrong parsing a record at offset 0x{record.offset:0>6x} in file {self.name}')
            # Sort any newly added records
            self.sort_records()
            late_records = [record for record in self.data_records if record.delay_parsing]

    def merge_records(self) -> None:
        # Iterate data records in order
        index: int = 0
        refresh_merged = False
        while index < len(self.data_records) - 1:
            record: DataRecord = self.data_records[index]
            next_record: DataRecord = self.data_records[index + 1]
            if self.can_merge(record, next_record):
                # Merge next data record into current data record
                record.merge(next_record)
                refresh_merged = True
            else:
                if refresh_merged:
                    record.refresh_rom_data()
                    refresh_merged = False
                index += 1

    def can_merge(self, record: DataRecord, next_record: DataRecord) -> bool:
        if record.offset + record.length < next_record.offset:
            return False
        if record.type == RecordType.CamPosData and next_record.type == RecordType.CamPosData:
            return True
        if record.type == RecordType.Vtx and next_record.type == RecordType.Vtx:
            return True
        return False

    def adjust_record(self, record: DataRecord, offset: int, length: int) -> None:
        record.offset = offset
        record.length = length
        record.data = record.file.rom.read_bytes(
            record.start + record.offset, record.length)

    # Add unknown record at file end
    def add_unknown_record_at_file_end(self) -> None:
        # Handle data at the end of the file
        last_record = self.data_records[-1]
        if last_record.length == -1:
            raise Exception(f'Last record ({last_record.type.value}) of unknown size for file {self.name} starting at 0x{last_record.offset:0>8x}')
        else:
            last_record_end_offset = last_record.offset + last_record.length
            last_record_end = self.start + last_record_end_offset
            data_record = DataRecord(self, last_record_end_offset, self.end - last_record_end)
            data_record.align = 8

    def check_for_overlapping_records(self) -> None:
        count = len(self.data_records)
        for i in range(0, count - 1):
            record = self.data_records[i]
            next_record = self.data_records[i + 1]
            if record.offset + record.length > next_record.offset:
                raise Exception(f'Overlapping records: {record.type.value} at offset 0x{record.offset:08X}, length 0x{record.length:0>8x} and {next_record.type.value} at offset 0x{next_record.offset:08X}, length 0x{next_record.length:0>8x} in {self.name}')

    # Add unknown records between sorted records
    def add_unknown_records(self) -> None:
        # Handle data between records
        index: int = len(self.data_records) - 1
        while index > 0:
            record = self.data_records[index]
            previous_record = self.data_records[index - 1]
            previous_record_end = align4(previous_record.offset + previous_record.length)
            if record.offset > previous_record_end:
                data_record = DataRecord(self, previous_record_end, record.offset - previous_record_end, store_in_file=False)
                data_record.align = 8
                self.data_records.insert(index, data_record)
            index -= 1

    # Add and return the given record or return the existing one
    def add_record(self, records: list[DataRecord], record: DataRecord) -> DataRecord:
        existing_record: Optional[DataRecord] = self.get_existing_record(
            records, record)
        if existing_record is not None:
            return existing_record
        records.append(record)
        return record

    # Return the existing record matching the given record or None
    def get_existing_record(self, records: list[DataRecord], record: DataRecord) -> Optional[DataRecord]:
        existing_record: Optional[DataRecord] = next(
            (x for x in records if x.offset == record.offset), None)
        if existing_record is not None and existing_record != record:
            raise Exception(
                f'Existing {existing_record.type} {type(existing_record).__name__} at 0x{existing_record.offset:08X} does not match new {record.type} {type(record).__name__} at 0x{record.offset:08X} in {self.name}')
        return existing_record

    # Return the existing data record matching the given file offset or None
    def get_existing_record_by_offset(self, offset: int, record_type: RecordType) -> Optional[DataRecord]:
        existing_record: Optional[DataRecord] = next(
            (x for x in self.data_records if x.offset == offset), None)
        if existing_record is not None and existing_record.type != record_type:
            raise Exception(
                f'Existing {existing_record.type.value} at 0x{existing_record.offset:08X} does not match requested type {record_type} in {self.name}')
        return existing_record

    @overload
    def get_existing_record_by_vanilla_offset(self, offset: int, record_type: RecordType, throw_on_not_found: Literal[True]) -> DataRecord: ...
    @overload
    def get_existing_record_by_vanilla_offset(self, offset: int, record_type: Literal[RecordType.CutsceneData], throw_on_not_found: Optional[bool]) -> Optional[SceneCutsceneData]: ...
    @overload
    def get_existing_record_by_vanilla_offset(self, offset: int, record_type: Literal[RecordType.SceneHeader], throw_on_not_found: Literal[True]) -> SceneHeader: ...
    @overload
    def get_existing_record_by_vanilla_offset(self, offset: int, record_type: Literal[RecordType.RoomHeader], throw_on_not_found: Literal[True]) -> RoomHeader: ...

    # Return the existing data record matching the given file offset or None
    def get_existing_record_by_vanilla_offset(self, offset: int, record_type: RecordType, throw_on_not_found: bool = False) -> Optional[DataRecord]:
        existing_record: Optional[DataRecord] = next(
            (x for x in self.data_records if x.vanilla_offset == offset), None)
        if existing_record is None and throw_on_not_found:
            raise Exception(f'{record_type} at 0x{offset:08X} could not be found in parsed file records')
        if existing_record is not None and existing_record.type != record_type:
            raise Exception(
                f'Existing {existing_record.type.value} at 0x{existing_record.vanilla_offset:08X} does not match requested type {record_type} in {self.name}')
        return existing_record

    def get_existing_record_by_vanilla_offset_and_segment(self, offset: int, record_type: RecordType, segment: int) -> DataRecord:
        return self.get_existing_record_by_vanilla_offset(offset, record_type, True)

    @overload
    def get_existing_records_by_type(self, record_type: Literal[RecordType.TransitionActorList]) -> list[SceneTransitionActorList]: ...
    @overload
    def get_existing_records_by_type(self, record_type: Literal[RecordType.PathList]) -> list[ScenePathList]: ...
    @overload
    def get_existing_records_by_type(self, record_type: Literal[RecordType.Cams]) -> list[CollisionBgCamInfoList]: ...
    @overload
    def get_existing_records_by_type(self, record_type: Literal[RecordType.ActorList]) -> list[RoomActorList]: ...
    @overload
    def get_existing_records_by_type(self, record_type: Literal[RecordType.ObjectList]) -> list[RoomObjectList]: ...

    def get_existing_records_by_type(self, record_type: RecordType) -> list[DataRecord]:
        return list(filter(lambda r: r.type == record_type, self.data_records))

    def encode(self, unit_test_alignment: bool = False) -> bytearray:
        # Re-sort records to handle any insertions.
        self.sort_records()
        # Resize data records. Assumes records are sorted by offset.
        # Don't build full encode at first as pointers may shift.
        offset: int = 0
        i = 0
        num_records = len(self.data_records)
        while i < num_records:
            record = self.data_records[i]
            record.offset = offset
            record.data = record.encode()
            record_length = len(record.data)
            record.length = record_length
            if i + 1 < num_records - 1:
                next_record = self.data_records[i + 1]
                # Padding bytes are read in as Unknown records between known record types.
                # When changing known records in a way that affects alignment, take away
                # existing padding bytes if needed.
                padding_record = None
                if next_record.type == RecordType.Unknown and next_record.is_empty():
                    if i + 2 < num_records - 1:
                        padding_record = next_record
                        next_record = self.data_records[i + 2]
                if next_record.align == 4 or (next_record.type == RecordType.Unknown and unit_test_alignment):
                    record.length = align4(offset + record_length) - offset
                elif next_record.align == 8:
                    record.length = align8(offset + record_length) - offset
                elif next_record.align == 16:
                    record.length = align16(offset + record_length) - offset
                if padding_record is not None:
                    alignment_padding_length = record.length - record_length
                    if padding_record.length <= alignment_padding_length:
                        self.data_records.pop(i + 1)
                        num_records -= 1
                    else:
                        padding_record.length -= alignment_padding_length
                        padding_record.data = bytearray(int.to_bytes(0, padding_record.length, 'big'))
            offset += record.length
            i += 1
        # All pointers are set, output the file raw bytes
        bytes = bytearray()
        for record in self.data_records:
            record_bytes = record.encode()
            while len(record_bytes) < record.length:
                record_bytes.extend(int.to_bytes(0, 1, 'big'))
            bytes.extend(record_bytes)
        return bytes

    # Two-stage process to write to rom.
    # Start/end of all files must be determined prior to
    # final encode as other files may reference this file's
    # location, such as scene files referencing their room files.
    def update_start_and_end(self, start_address: Optional[int] = None) -> int:
        raw_file = self.encode()
        if start_address is None:
            new_start = align_file(self.start)
        else:
            new_start = align_file(start_address)
        new_end = new_start + len(raw_file)
        self.start = new_start
        self.end = new_end
        return align_file(new_end)

    def write(self, rom: Rom) -> int:
        raw_file = self.encode()
        if self.name in SCENE_EXTERNAL_REFERENCES.keys():
            for record_type, offset, data_references, code_references in SCENE_EXTERNAL_REFERENCES[self.name]:
                record = self.get_existing_record_by_vanilla_offset(offset, record_type)
                if record is None:
                    raise Exception(f'Offset {offset:0>6x} does not match any records in {self.name}')
                record_address = record.get_segment_address_bytes()
                for external_pointer_address in data_references:
                    self.write_external_data_pointer(rom, record_address, external_pointer_address)
                for external_pointer_addresses in code_references:
                    self.write_external_code_pointer(rom, record_address, external_pointer_addresses)
        file_length = len(raw_file)
        while file_length < align_file(file_length):
            raw_file.extend(int.to_bytes(0, 1, 'big'))
            file_length += 1
        rom.write_bytes(self.start, raw_file, True)
        # Always supply the vanilla start address in case writing occurs multiple times for some reason.
        # DMA changes are patched by comparing with the original file specified with `from_file`.
        # DMA key is always the current file start address, requiring tracking of the DMA start address
        # independent of the vanilla file address and the proposed start address. This is why there there
        # are 3 start addresses stored for this class:
        #   start: new file start to be written to rom
        #   rom_start: current DMA key for this file, updated to new file start after DMA table is changed
        #   vanilla_start: original file start from an unmodified rom
        rom.update_dmadata_record_by_key(self.rom_start, self.start, self.end, self.vanilla_start)
        self.rom_start = self.start
        return align_file(self.end)

    def write_external_data_pointer(self, rom: Rom, record_address: bytearray, external_pointer_address: int) -> None:
        rom.write_bytes(external_pointer_address, record_address)

    def write_external_code_pointer(self, rom: Rom, record_address: bytearray, external_pointer_addresses: tuple[int, int]) -> None:
        address = int.from_bytes(record_address, 'big')
        address_low: int = address & 0xFFFF
        address_high: int = (address >> 16) + (1 if address_low > 0x7FFF else 0)
        external_high, external_low = external_pointer_addresses
        rom.write_bytes(external_high, address_high.to_bytes(2, 'big'))
        rom.write_bytes(external_low, address_low.to_bytes(2, 'big'))

    # Return the file data as a serializable dict
    def to_json(self) -> dict[str, Any]:
        return {
            'version': self.version,
            'type': self.type.value,
            'name': self.name,
            'start': f'{self.start:08X}',
            'end': f'{self.end:08X}',
            'vanilla_start': f'{self.vanilla_start:08X}',
            'records': [x.to_json() for x in self.data_records],
            'parsed': self.parsed,
        }

    # Does not include other parsed data from the Scene/Room subclasses.
    # Only use for files with pure data.
    @staticmethod
    def from_json(rom: Rom, cache: dict[str, Any]) -> FileDataRelocator:
        if cache['version'] != FileDataRelocator.version:
            raise SceneCacheFileException(cache['name'], cache['type'], cache['version'], FileDataRelocator.version)
        file = FileDataRelocator(rom, cache['name'], cache['start'], cache['end'], RecordType(cache['type']))
        file.vanilla_start = cache['vanilla_start']
        file.data_records = [DataRecord.from_json(file, x) for x in cache['records']]
        file.parsed = cache['parsed']
        return file
