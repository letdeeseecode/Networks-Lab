# Networks-Lab
Solutions/Demonstrations of various Networking Problems/Assignments

Assignment 1: Write a program to run TCP client and server socket programs where client first says “Hi” and in response server says “Hello”.
.
![output_of_assignment1](https://github.com/letdeeseecode/Networks-Lab/blob/e6545e5f8f57e6192589aabbc7b344fb77c3c84d/assignment_1/output_1.png)

Assignment 2: Write a program using TCP socket to implement the following:
i. Server maintains records of fruits in the format: fruit-name, quantity Last-sold,
(server timestamp),
ii. Multiple client purchase the fruits one at a time,
iii. The fruit quantity is updated each time any fruit is sold,
iv. Send regret message to a client if therequested quantityof the fruit is not
available.
v. Display the customer ids <IP, port> who has done transactions already. This
list should be updated in the server every time a transaction occurs.
vi. The total number of unique customers who did some transaction will be
displayed to the customer every time.
.
![output_of_assignment2](https://github.com/letdeeseecode/Networks-Lab/blob/cbf1ca6fd0cdc3b530a61227b65aa5d491cfdec3/assignment_2/output2.png)

Assignment 3: Same as Assignment 2 using UDP socket.
.
![output_of_assignment3](https://github.com/letdeeseecode/Networks-Lab/blob/26264d33d3ca0f668adf62bbfa0f92c358ee5676/assignment_3/output3.png)

Assignment 4: Install wireshark in a VM (Virtual Machine) environment. Draw a time diagram to show the steps in the protocols recorded in the captured file (saved in the .pcap file of wireshark) during a PING operation. List the L2, L3, L4 header fields that can be extracted from the .pcap file.
.
![output_of_assignment4](https://github.com/letdeeseecode/Networks-Lab/blob/2b5cef032f4d4342f89435f372ec3ebf65b1ad7a/assignment_4/output4.png)

Time Diagram:
![output_of_assignment4](https://github.com/letdeeseecode/Networks-Lab/blob/8f013fe98be2eaa73fefdd1a3b68d231c9cce6ab/assignment_4/output4_time_diagram1.png)
![output_of_assignment4](https://github.com/letdeeseecode/Networks-Lab/blob/91960faab6ac7e31467ebc9aa3d4897017f19a2d/assignment_4/output4_time_diagram2.png)

L2 Header Fields:
![output_of_assignment4](https://github.com/letdeeseecode/Networks-Lab/blob/69c8006d2971d6875380c177c6b2e2ea573a5c56/assignment_4/output4_L2_header.png)

L3 Header Fields:
![output_of_assignment4](https://github.com/letdeeseecode/Networks-Lab/blob/fe01485cfa8b34bfeac767ef0124a3d8b63b118c/assignment_4/output4_L3_header.png)

L4 Header Fields:
![output_of_assignment4](https://github.com/letdeeseecode/Networks-Lab/blob/1d496925ae805801e18b0fa1206555c59eaa0a7c/assignment_4/output4_L4_header.png)

Assignment 5: Learn and use maximum number of packet generation tools. Steps/ Hints: Install Iperf, D-ITG etc. and send traffic among mininet hosts.
.
![output_of_assignment5]()

Assignment 6: Develop a simple C based network simulator to analyze TCP traffic. Steps/ Hints: 1. Use TCP/IP based C libraries including socket to listen to the incoming packets at the Ethernet port. 2. Extract header and data of the incoming packets.
.
![output_of_assignment6](https://github.com/letdeeseecode/Networks-Lab/blob/467eff7fbd42495d4ac3c21af5c68e5a02edb600/assignment_6/output6_listening.png)
![output_of_assignment6](https://github.com/letdeeseecode/Networks-Lab/blob/25d9def78ac33c48a1b5460d049c38ef234c50ef/assignment_6/output6_headerinfo.png)

Assignment 7:Write UDP client server socket program where client sends one/two number(s)(integer or floating point) to server and a scientific calculator operation (like sin,cos,*,/, inv etc.) and server responds with the result after evaluating the value of operation as sent by the client. Server will maintain a scientific calculator. Detect in the mininet hosts with wireshark if there is any packet loss.
.
![output_of_assignment7](https://github.com/letdeeseecode/Networks-Lab/blob/a6ad30fc3d489670c5dbe4d4c1ffd667ce884969/assignment_7/output7.png)

Assignment 8: Write a program in C using thread library and TCP sockets to build a chat server which enable clients communicating to each other through the chat server. Message logs must be maintained in the server in a text file. Each client will see the conversations in real time. Clients must handled by a server thread. (Keep it like a group chatbox)
.

![output_of_assignment8](https://github.com/letdeeseecode/Networks-Lab/blob/e50aba2408a07990c16ed9030dd8fb6a103948be/assignment_8/output8.png)
![output_of_assignment8](https://github.com/letdeeseecode/Networks-Lab/blob/a750ffaac60558bd093a047937683d27de64533a/assignment_8/output8_logs.png)
