# Chat room system in console - Linux
#### Made by: Kopacz Mădălina-Elena (@MadalinaKopacz), Năstase Matei-Dorian (@matei-dorian) and Negruț Maria-Daniela (@NMDMaria). <br> Please contact us if you plan on using this code. </br> 
## About
This was a group project in our Operating Systems class. Finished and presented in January 2022.</br>

It's a chat system with rooms made for short-message broadcasting made for ***Linux distributions***, so far **only on localhost**. Using a **tree system** the messages from a parent are propagated to all indirect childs. The server has been integrated in a **daemon** and has a **dynamyc library**.</br>

For the clients, you can use our menu and connect as either one. After choosing a type of client you'll be asked to **enter a path** which should follow the format: ***room_name/another_room_name/.../final_room_name*** (room names separated with slash)</br>
After this you'll recieve a connected message from the server, also the last message found in this room. After this you can chat freely!</br>
When you want to end the connection, you can enter '#' in the chat.<br>

![Example when being the first one to connect.](https://i.gyazo.com/5abe1d9018ff227203030e566f67a3de.png)
![Example when connecting to an indirect child of a channel](https://i.gyazo.com/fde354415fcd0160264ec8ff2745eb6b.png)

## Compiling
### Server side:
1. Go to ```<your download path>/chatsystem/libs/serverlib``` and in a terminal write:
  ```
  gcc -c -Wall -Werror -fpic server.c
  gcc -shared -o libserver.so server.o
  ```
2. Go to ```<your download path>/chatsystem/programs/server``` and in terminal write:
  ```
  gcc -L/<your download path>/chatsystem/libs/serverlib - Wl,-rpath=/<your download path>/chatsystem/libs/serverlib - Wall -o server server.c -lserver
  ```
3. Make the daemon work at reboot. Open in a terminal ``` crontab -e ``` and after selecting your prefered text editor go to the end of file and write:
  ```
  @reboot .<your download path>/chatsystem/programs/server
  ```
Now you just have to compile the client side and reebot.</br>
### Client side:
1. Go to ```<your download path>/chatsystem/libs/clientlib``` and in a terminal write:
  ```
  gcc -c -Wall -Werror -fpic client.c
  gcc -shared -o libserver.so client.o
  ```
2. Go to ```<your download path>/chatsystem/programs/client``` and in terminal write:
  ```
  gcc -L/<your download path>/chatsystem/libs/clientlib - Wl,-rpath=/<your download path>/chatsystem/libs/clientlib - Wall -o sublisher sublisher.c -lclient
  gcc -L/<your download path>/chatsystem/libs/clientlib - Wl,-rpath=/<your download path>/chatsystem/libs/clientlib - Wall -o publisher publisher.c -lclient
  gcc -L/<your download path>/chatsystem/libs/clientlib - Wl,-rpath=/<your download path>/chatsystem/libs/clientlib - Wall -o subscriber subscriber.c -lclient
  ```
### Compile the menu
1. Open the menu.c file and at [line 13](https://github.com/NMDMaria/Chat_system/blob/main/menu.c#L13) and make sure that the path leads to where you installed the program.
2. Go to ```<your download path>``` and open a terminal and write:
  ```
  gcc menu.c -o menu
  ```
3. Open a terminal where you downloaded the project and open the menu!
  ```
  ./menu
  ```

## How to use
The **client types** are:
- **Publisher** - can **only broadcast/send messages** without seeing any messages sent by other user
- **Subscriber** - can **only see the messages**
- **Sublisher** - a combination of both, can **send & recieve messages**. Only downside: you have to send a message to recieve all the messages (if you have a solution for this contact us!)
</br>

After choosing the client type you'll encounter the message ```Introduce path:```. The format for the path is ***room_name/another_room_name/.../final_room_name***. You can have 2 or more clients connected to the same channel and maximum of 30 users connected in the chat system at the same time. 
</br>

Send as many messages you want and see how the system propagates them!<br>
When you are ready to **exit**, if you are a sublisher or publisher you can **enter # in the chat**. As a subscriber you'll have to stop the program with Ctrl + C (not so gracious, we know.)
