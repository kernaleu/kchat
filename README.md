# kChat — Kernal Chat
**kChat** or Kernal Chat is a functional chat-room server that you can connect using netcat, telnet or any TCP client.

Joining the chat is as simple as running `nc server port`.

## Features
* Requires no external libraries
* Register/login (SHA-256)
* Rules (much like iptables, enables users to choose who to receive from and who to send to.)
* Direct messages
* MOTD
* Uses I/O multiplexing (no threads)
* Everything else is configurable in `config.c`

However, not having a specific client comes with some limitations. Such as overlapping issues, no arrow history etc.\
To make your kernal-chat experience better, we recommend using [rlwrap](https://github.com/hanslub42/rlwrap) and connecting with the command below,

`rlwrap -S "> " nc server port`

Still not excited?\
Join our kChat instance available at `chat.kernal.eu 1337` or `kernalsblyat57vz.onion 1337`.
