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
Output for IPERF (TCP):
![output_of_assignment5](https://github.com/letdeeseecode/Networks-Lab/blob/fa842ab45b081379c701f06e8c5e544d7304958c/assignment_5/output5_iperf_tcp.png)

Output for IPERF (UDP):
![output_of_assignment5]()

Output for D-ITG:
![output_of_assignment5]()

Assignment 6: Develop a simple C based network simulator to analyze TCP traffic. Steps/ Hints: 1. Use TCP/IP based C libraries including socket to listen to the incoming packets at the Ethernet port. 2. Extract header and data of the incoming packets.
.
![output_of_assignment6](https://github.com/letdeeseecode/Networks-Lab/blob/467eff7fbd42495d4ac3c21af5c68e5a02edb600/assignment_6/output6_listening.png)
![output_of_assignment6](https://github.com/letdeeseecode/Networks-Lab/blob/25d9def78ac33c48a1b5460d049c38ef234c50ef/assignment_6/output6_headerinfo.png)

Assignment 7: Write UDP client server socket program where client sends one/two number(s)(integer or floating point) to server and a scientific calculator operation (like sin,cos,*,/, inv etc.) and server responds with the result after evaluating the value of operation as sent by the client. Server will maintain a scientific calculator. Detect in the mininet hosts with wireshark if there is any packet loss.
.
![output_of_assignment7](https://github.com/letdeeseecode/Networks-Lab/blob/a6ad30fc3d489670c5dbe4d4c1ffd667ce884969/assignment_7/output7.png)

Assignment 8: Write a program in C using thread library and TCP sockets to build a chat server which enable clients communicating to each other through the chat server. Message logs must be maintained in the server in a text file. Each client will see the conversations in real time. Clients must handled by a server thread. (Keep it like a group chatbox)
.

![output_of_assignment8](https://github.com/letdeeseecode/Networks-Lab/blob/e50aba2408a07990c16ed9030dd8fb6a103948be/assignment_8/output8.png)
![output_of_assignment8](https://github.com/letdeeseecode/Networks-Lab/blob/a750ffaac60558bd093a047937683d27de64533a/assignment_8/output8_logs.png)

Assignment 9: Write a client server socket program in TCP for uploading and downloading files between two different hosts. Also calculate the transfer time in both the cases.
.
![output_of_assignment9](https://github.com/letdeeseecode/Networks-Lab/blob/7dc1ff700e3cc145e7a65aa338502014af2f4127/assignment_9/output9.png)

Assignment 10: Write two C programs using raw socket to send i. TCP packet where TCP payload will contain your roll number. ii. ICMP time stamp messages towards a target IP
.
Output (i):
![output_of_assignment10i](https://github.com/letdeeseecode/Networks-Lab/blob/ad6e8dbd22e7911b66db1ecfeff18a7c03c0ba6d/assignment_10/output10_roll_payload.png)

Output (ii)
![output_of_assignment10ii](https://github.com/letdeeseecode/Networks-Lab/blob/8e1b0302cb466d3df26b53bb11e41f20d63fa796/assignment_10/output10_icmp.png)
![output_of_assignment10ii](https://github.com/letdeeseecode/Networks-Lab/blob/4417d71e8a193f3a2869f096a436232bf521ee1d/assignment_10/output10_req.png)
![output_of_assignment10ii](https://github.com/letdeeseecode/Networks-Lab/blob/6997a5172e05851e5625db482513e7d83c71802e/assignment_10/output10_reply.png)

Assignment 11: Write a RAW socket program to generate TCP SYN flood based DDoS attack towards an IP address. Take four mininet hosts as agent devices.
.
![output_of_assignment11](https://github.com/letdeeseecode/Networks-Lab/blob/e7520c5dc54babcf2ca6eaa2c9804e213a383bef/assignment_11/output11.png)

Assignment 12: Same as Assignment 10 with ICMP packets using RAW sockets
.
![output_of_assignment12](https://github.com/letdeeseecode/Networks-Lab/blob/e85f51da9f010f0486c00a25253ee79a79b979f0/assignment_12/output12.png)

Assignment 13: Create a binary tree topology with 7 switches in mininet. Capture packets at the root switch. Write a C program to extract the headers and draw a time diagram to show the protocols displayed in the captured file (save the .pcap/.pcapng file of wireshark/tshark) during a PING operation. List the L2, L3, L4 protocols that can be extracted from the .pcap/.pcapng file.
.
![output_of_assignment13](https://github.com/letdeeseecode/Networks-Lab/blob/8084d50f425fbe75ac8a3f6ce69efcbdf1616f5e/assignment_13/output13_ping.png)
![output_of_assignment13](https://github.com/letdeeseecode/Networks-Lab/blob/ab6d51a0d7c49e16044899a73dd8413ce150eee7/assignment_13/output13_pinganalyzer.png)

Assignment 14: Create a custom leaf-spine topology in mininet using python which can be scaled with increasing switch radix.
.
![output_of_assignment14](https://github.com/letdeeseecode/Networks-Lab/blob/197f36312b8b43be2315a6eabdf39868768e0793/assignment_14/output14.png)
