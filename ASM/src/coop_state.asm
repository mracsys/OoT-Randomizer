COOP_CONTEXT:

COOP_VERSION:
.word 7 ; Increment this if layout of co-op state changes

PLAYER_ID:
.byte 0x00 ; Written by frontend
PLAYER_NAME_ID:
.byte 0x00
INCOMING_PLAYER:
.halfword 0x0000
INCOMING_ITEM:
.halfword 0x0000
MW_SEND_OWN_ITEMS:
; Written by multiworld plugin. If nonzero, the OUTGOING fields are set even if
; we find our own item, for the plugin's information.
.byte 0x00
MW_PROGRESSIVE_ITEMS_ENABLE:
; Written by multiworld plugin. If nonzero, item upgrades for outgoing items
; will be based on MW_PROGRESSIVE_ITEMS_STATE.
.byte 0x00
.align 4

.word 0x00000000 ; Padding where OUTGOING_KEY used to be.
OUTGOING_ITEM:
.halfword 0x0000
OUTGOING_PLAYER:
.halfword 0x0000

.area 8*256, 0xDF
PLAYER_NAMES:
.endarea

.area 5, 0x00
CFG_FILE_SELECT_HASH:
.endarea
.align 4

.area 4*256, 0x00
MW_PROGRESSIVE_ITEMS_STATE:
.endarea

.align 4
OUTGOING_KEY:
.word 0x00000000
.word 0x00000000

.align 4
SERIAL_CONTEXT:
SERIAL_VERSION:
.byte 1
SERIAL_ENABLE:
.byte 0
SERIAL_PROCESSING:
.byte 0
.align 4
.area 32, 0x00
; SerialVirtualDevice struct, structure in usb.h
EMULATOR_DEVICE:
.endarea

; 32-byte alignment required for the Wii's USB DMA engine
; Buffer sizes are 512 byte payload plus 32 bytes for
; flashcart-specific metadata
.align 32
.area 544, 0x00
SERIAL_RECEIVE_BUFFER:
.endarea
.area 544, 0x00
SERIAL_TRANSMIT_BUFFER:
.endarea

.align 4
