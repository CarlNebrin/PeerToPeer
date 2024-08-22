1. Two peers connect to a relay server.
2. Each peer send an integer message to the relay server.
3. The relay server sends the message of peer A to peer B and vice versa.
4. The peers try to directly send their own message to each other, and receive a message from the other. The IP and port of the peers is assumed to be preknown.
5. After a number of amount of sends and receive, the program assumes holepunching and a handshake has occured, and verifies if the directly received message from the other peer is the same as the one received from the relay server.
