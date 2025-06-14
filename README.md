# MTC: Scalable Transaction Commit for Multi-Master Cloud Databases

![Architecture](https://github.com/user-attachments/assets/911b802a-eb5e-47c2-bf7d-0f3206b2202b)


This repository contains the prototype implementation for the research paper "MTC: Scalable Transaction Commit for Multi-Master Cloud Databases." MTC is a novel transaction commit protocol designed to enhance write scalability and performance in multi-master cloud databases that use a compute-storage disaggregated architecture. It addresses the bottlenecks of traditional systems by enabling efficient, consistent writes across multiple master nodes while ensuring sequential consistency.

## Core Features
 * *Check-then-Replicate Workflow*: MTC introduces an efficient "check-then-replicate" paradigm, which decouples global transaction ordering from log synchronization. Transactions are validated locally first, and only committed logs are broadcast asynchronously to other nodes, saving network bandwidth and reducing commit latency.
 * *Asynchronous Global Ordering*: Master nodes asynchronously prefetch Global Transaction Sequence Numbers (GTSNs) from a central generator and assign them locally to transactions upon commit. This design effectively hides network latency, preventing the commit phase from being blocked by network round-trips.
 * *Pipelined Row-Level Conflict Detection*: MTC performs fine-grained, row-level conflict detection against a local replica of the Multi-Master Committed Transaction Log (MCTL). The detection process is pipelined to overlap computation with network communication, hiding the latency of fetching missing logs.
 * *Non-Intrusive Plugin*: The system is implemented as a non-intrusive plugin for MySQL, making it adaptable without requiring fundamental changes to the underlying storage engine or specialized hardware.

## Repository Structure
This repository is organized into two main parts:

 * *Percona-MTC-master/*: This directory contains the main prototype system, implemented as a plugin on Percona Server for MySQL. The script/ subdirectory provides essential scripts for building, initializing, and running the MTC-enabled database nodes.
 * *components/*: This directory contains standalone modules used by the MTC system or for experimental evaluation:
   * *id-increment*: The centralized Global Transaction Sequence Number (GTSN) generator service that provides unique, monotonically increasing transaction IDs.
   * *centralized-lock-table:* An implementation of a traditional cross-node page lock mechanism, used as a pessimistic locking baseline for performance comparison in our experiments.
   * *multi_net_io, easy_logger*: Utility libraries for asynchronous network communication and logging, respectively.
