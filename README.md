# Kernal Chat
A full-blown chatroom that you can connect using only netcat. No, really. We're not kidding here.
Or any other TCP client. Yup.

However, not having a specific client comes with some limitations. To fix the overlapping issue we recommend using rlwrap.

`rlwrap -S "> " nc chat.kernal.xyz 1337`
You can customize the prompt:
`rlwrap -S "OwO: " nc chat.kernal.xyz 1337`
