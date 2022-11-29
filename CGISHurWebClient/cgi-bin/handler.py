#!/usr/bin/env python3

print("Content-Type: text/html")
print()

print("""<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Web client</title>
</head>
<body>
    <h3>Web socket client</h3>
    <form action="/cgi-bin/handler.py">
        Enter your message: <input type="text" name="msg_txt"><br />
        Enter client's id: <input type="text" name="client_id"><br />
        <button type="submit" value="send_id" name="action">Send to one</button>
        <button type="submit" value="send_all" name="action">Send to all</button>
        <button type="submit" value="disconnect" name="action">Disconnect</button>
    </form>
</body>
<table>
    <thead>
        <tr>
            <th>From</th>
            <th>Message</th>
        </tr>
    </thead>
    <tbody>
""")

import cgi

form = cgi.FieldStorage()
action = form['action'].value

if action == 'send_id':
    msg_txt = form.getfirst("msg_txt", "none")
    client_id = form.getfirst("client_id", "none")
elif action == 'send_all':
    msg_txt = form.getfirst("msg_txt", "none")
    print("<tr><td>" + "MR_ALL" + "</td><td>" + msg_txt + "</td></tr>")

print("""</tbody>
        </table>
        </html>>""")