# Kernal Chat
KChat or Kernal Chat is a functional chatroom that you can connect using netcat or any TCP client.

Joining the chat is as simple as running `nc server port`

However, not having a specific client comes with some limitations. Such as overlapping issues, no arrow history etc.
To make your kernal-chat experience better, we recommend using rlwrap and connecting with the command below,

`rlwrap -S "> " nc server port`