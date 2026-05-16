Send to Emulator
----------------------
Client:
Maintain queue of messages to send,
only sending one at a time.
Emulator:
1. Check MSG_PROCESSING = 0
2. Set device receiving = 1
3. Write message to data buffer
4. Write header to device receive_header
5. Set MSG_PROCESSING = 1
6. Set device receiving = 0
ROM:
7. Check device receiving = 0
8. Check device receive_header != 0
9. Process message as usual
10. Set receive_header = 0
11. Set MSG_PROCESSING = 0


Send from Emulator
----------------------
Client:
Check incoming messages before
processing the next message to send
to ROM. Immediately queue required
responses, if any, before any pending
messages.
Emulator:
1. Check device transmit_header != 0
2. Check device transmit_addr != 0
3. Check device busy = 1
4. Read specified memory from ROM
5. Set device busy = 0
ROM:
1. Set device transmit_addr
2. Set device transmit_header
3. Set device busy = 1
4. Loop until device busy = 0


Hint Message Format
----------------------
SEND_HINT , GOSSIP_STONE_ID (2 bytes) , HINT_TYPE_ID (1 byte) , HINT_DATA

Hint Type Data Formats
WOTH (1 byte): HINT_AREA
GOAL (3 bytes): HINT_AREA, GOAL_WORLD, GOAL_TEXT
FOOLISH (1 byte): HINT_AREA
ITEM (4 bytes): HINT_AREA, ITEM_WORLD, ITEM_ID
LOCATION (min 12 bytes, 11 bytes per loc): NUM_LOCATIONS, [LOCATION_OVERRIDE_KEY, ITEM_WORLD, ITEM_ID] for each hinted location
ENTRANCE (4 bytes): ENTRANCE, EXIT
MAJOR_ITEM (2 bytes): HINT_AREA, ITEM_COUNT