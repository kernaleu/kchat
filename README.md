# Kernal Chat
A full-blown chatroom that you can connect using only netcat. No, really. We're not kidding here.
Or any other TCP client. Yup.

However, not having a specific client comes with some limitations. To fix the overlapping issue we recommend using rlwrap.

`rlwrap -S "> " nc localhost 1337`

You can customize the prompt:

`rlwrap -S "OwO: " nc localhost 1337`

Enter `@username` or `@username:password` to get started.

Upload files:
`cat photo.png | nc -c localhost 1337`

You can also upload output of commands:
`ls -la | nc -c localhost 1337`

Download files:
`echo '$hello' | nc localhost 1337 > file.png`
`echo '$hello' | nc localhost 1337 | feh -`

I think you can think of other ways to use this :)
