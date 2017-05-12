# unix_papayoo

This project is entirely conceived with C language.
It's a multiplayer game version of the cards game "Papayoo".
It has been developed for academic purpose, to learn sockets and ipcs (system V's semaphores / shared memory) in particular
and to improve our general unix system calls knowledge.

Special features to mention :
-lock file (2 instacens of a the server executables cannot ben run at once).
-file stream dupping (stderr can be optionnaly redirect to a log file).
-signals handling (SIGINT, SIGTERM, SIGQUIT) catching for clean exit (both client and server side).
-ipcs killing (no dirty ipcs remain after execution)
- system V semaphores (used with Courtoi's algorithm to protect the access to the shared memory)
-sockets implementation (to make client and server communicating through network layer)
