# Distributed Email System
A fault-tolerant distributed email system using Anti-Entropy method with eventual path propagation. There are $N$ server replicas, with $N=5$ by default. Note that a [Spread](http://spread.org/) daemon must be run at the same host where a server is run.

The mail server is started with its id as a parameter:
$\qquad$$ ./server [1-5]
    
The mail client can be run with simply:
$\qquad$$ ./client

## Client functionalities
The email system provides the following capabilities; each capability is one option of a client program's top level menu:
1. "u _<user name>_": Login with _<user name>_. 
2. "c _<id>_": Connect to Server _<id>_. The client needs to connect to a mail server to perform any of the tasks below.
3. "l": List the headers of received mail.
4. "m": Mail a message to a user. Subsequently, enter:
   $\qquad$ to:  _<user name>_
   $\qquad$ subject:  _<subject string>_
   $\qquad$ _<msg string>_
5. "r $i$": Read the $i^{th}$ message in the current list of messages.
6. "d $i$": Delete the $i^{th}$ message in the current list of messages.
7. "v": Print the membership (identities) of the mail servers in the current mail server's network component. 
