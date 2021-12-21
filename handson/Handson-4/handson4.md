# Hands-on-4: TCP

> **Q1:** What are the IP addresses and TCP ports of BingBing and DongDong?

For DongDong(Sender):

+ **IP Address** is **128.30.4.222**;

+ **TCP port** is **39675**.

For BingBing(Receiver):

+ **IP Address** is **128.30.4.223**;

+ **TCP port** is **5001**.

> **Q2:** How many KB were transferred during this TCP session and how long did it last?

Notice that there is a record:

> 20:34:44.320952 IP 128.30.4.222.39675 > 128.30.4.223.5001: Flags [.], seq 1566225:1567673, ack 1, win 115, options [nop,nop,TS val 282139320 ecr 282204936], length 1448

So the **1567672KB** were transferred during this TCP session.

It last about **2.865** seconds.

> **Q3:** What is the throughput (in KB/s) of this TCP flow between DongDong and BingBing?

The throughput of this TCP flow is 1567672 / 2.865 ≈ **547180.45 KB/s**

> **Q4:** What is the round-trip time (RTT) between DongDong and BingBing?

Three handshakes:

> 20:34:41.473518 IP 128.30.4.222.39675 > 128.30.4.223.5001: Flags [S], seq 1258159963, win 14600, options [mss 1460,sackOK,TS val 282136473 ecr 0,nop,wscale 7], length 0

> 20:34:41.474055 IP 128.30.4.223.5001 > 128.30.4.222.39675: Flags [S.], seq 2924083256, ack 1258159964, win 14480, options [mss 1460,sackOK,TS val 282202089 ecr 282136473,nop,wscale 7], length 0

> 20:34:41.474079 IP 128.30.4.222.39675 > 128.30.4.223.5001: Flags [.], ack 1, win 115, options [nop,nop,TS val 282136474 ecr 282202089], length 0

RTT is measured as a delay between SYN and ACK packets – between the first and second packet sent by the client.

So the RTT is 0.474079s - 0.473518s = **0.561 ms**

