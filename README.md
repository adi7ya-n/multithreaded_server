# multithreaded_server
Tic-Tac-Toe game server built using the Boost.Asio library. 

# Design:
* Server creates a thread pool, with multiple workers executing io_context.run(). There are 2 kinds of jobs posted to the io_context manager: Connection & Data.
  Connection packets are used to retrieve usernames and to indicate which player goes first and data packets are used to relay moves between players.
* Asio's async_accept() function handles incoming connections, and creates a handler for each player.
* Matchmaking is performed in the main thread.
* Game objects take ownership of the player handlers. 

