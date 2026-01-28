
HSDECT Project
############

The **HSDECT Project (HS-DECT-2020)** is a research-oriented implementation framework for **DECT-2020 NR** on the Nordic **nRF91x1** platform.
It is developed at **Hochschule Aalen** and is intended to support **experimental MAC-layer research**, performance evaluation, and cross-layer optimization on top of the Nordic DECT NR+ PHY.

The project deliberately avoids implementing a full ETSI-compliant DECT NR+ protocol stack.
Instead, it exposes the PHY capabilities provided by Nordic’s modem firmware and builds a **custom, modular MAC layer** on the application processor.
This design allows controlled experimentation with scheduling, medium access, reliability mechanisms, and multi-UE behavior.

Motivation
**********

DECT-2020 NR is positioned as a low-complexity, icense-exempt spectrum alternative for private wireless networks.
However, most available implementations tightly couple PHY and MAC behavior, limiting research flexibility.

The HDECT Project addresses this gap by:

* Treating the DECT NR+ PHY as a **black-box radio service**
* Implementing MAC functionality entirely in application software
* Providing runtime control via a shell interface
* Enabling repeatable measurements and experiments

This approach allows direct comparison with MAC concepts known from **private 5G**, **Wi-Fi**, and other cellular systems.

Design Goals
************

The main design goals of the HDECT Project are:

* **Modularity**  
  Clear separation between PHY access, MAC logic, scheduling, measurement, and control

* **Research flexibility**  
  Easy modification of MAC behavior without modem firmware changes

* **Observability**  
  Built-in performance measurement (latency, throughput, reliability)

* **Interactivity**  
  Runtime control via shell commands (``hdect``)

* **Reproducibility**  
  Support for scripted experiments and repeatable test scenarios

System Architecture
*******************

The HDECT Project follows a **dual-core architecture** as provided by the nRF91x1 platform:

Application Core
================

The application core runs **Zephyr RTOS** and hosts:

* Custom MAC layer logic
* Scheduling and timing control
* UE management and addressing
* Performance measurement (ping, perf)
* Shell interface and experiment control

Modem Core
==========

The modem core runs Nordic’s closed-source firmware and provides:

* DECT-2020 NR PHY
* Radio control and timing
* Carrier and MCS handling
* RSSI and reception metadata

Communication between the two cores is handled via the
:ref:`nrf_modem_dect_phy` interface.

PHY–MAC Split
*************

A key principle of the HDECT Project is the **strict separation of PHY and MAC responsibilities**.

Physical Layer (PHY)
====================

Provided by Nordic modem firmware:

* Radio access and synchronization
* Frame, slot, and subslot timing
* Modulation and coding (MCS)
* Transmission power control
* Reception metadata (RSSI, timing)

The PHY is accessed exclusively through a documented API and is not modified.

Medium Access Control (MAC)
===========================

Implemented entirely in the HDECT Project:

* Transmission and reception control
* Scheduling decisions
* Medium access strategies
* Reliability mechanisms (research extensions)
* Performance monitoring

This separation enables MAC-layer experimentation without violating firmware or regulatory constraints.

Scope of the Project
********************

The HDECT Project currently supports:

* Point-to-point and broadcast communication
* Interactive MAC control via shell
* Ping-based latency testing
* Continuous performance measurements
* Static scheduling (baseline)

The project is intentionally extensible and serves as a **foundation for advanced research**, including adaptive scheduling and machine-learning-based resource allocation.

Intended Use
************

The HDECT Project is intended for:

* Academic research and prototyping
* Master’s and PhD-level thesis work
* Experimental comparison with private 5G MAC designs
* Evaluation of DECT-2020 NR capabilities under controlled conditions

It is **not** intended for production deployment or standards compliance testing.




Modules
*******

The HS_DECT_2020 project is structured into the following modules (see :file:`src/`):

hs_core
=======

Core services shared by all features:

* Common utilities and helpers
* Shared data structures and configuration glue
* Common event/logging helpers used by MAC, ping and perf

hs_shell
========

Shell/CLI integration and command registration:

* Registers the root command: ``hdect``
* Defines subcommand trees: ``hdect mac``, ``hdect ping``, ``hdect perf``, ``hdect cfg``, ``hdect led``
* Initializes board LEDs at boot via ``SYS_INIT()``

.. note::

   Project uses the command name ``hdect`` (instead of ``dout``).

hs_hello
========

MAC-DECT “hello” / baseline traffic generator:

* Starts/stops a simple MAC-driven transmit/receive loop
* Sends a MAC text message payload over DECT
* Intended as a baseline for MAC behavior before adding advanced scheduling

hs_ping
=======

DECT ping utility (client/server style):

* Generates ping messages and measures basic RTT/response behavior
* Useful for connectivity checks and latency experiments

hs_perf
=======

Performance metrics collection and reporting:

* Starts/stops measurement sessions
* Resets counters/metrics
* Prints metrics summary (throughput/latency/jitter/loss depending on implementation)

hs_scheduler
============

Scheduling and timing control (research module):

* Central place for resource/scheduling policies (static first, adaptive later)
* Hooks for future MAC research: slot/subslot decisions, fairness, QoS, multi-UE scaling


Command Interface
*****************

The HS_DECT_2020 project exposes a Zephyr shell-based CLI rooted at:

* ``hdect``  – HS-DECT command tree

Root command
============

The command is registered as:

.. code-block:: c

   SHELL_CMD_REGISTER(hdect, &sub_hdect, "HS-DECT commands", NULL);

Available subcommands
=====================

LED control
-----------

.. code-block:: text

   hdect led on  <1-4>     # Turn LED on
   hdect led off <1-4>     # Turn LED off

MAC-DECT control
----------------

.. code-block:: text

   hdect mac start         # Start MAC-DECT app (t/r)
   hdect mac stop          # Stop MAC-DECT app
   hdect mac mes <text>    # Send MAC text message

This command group is registered as:

.. code-block:: c

   SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_mac,
       SHELL_CMD(start, NULL, "Start MAC-DECT app (t/r)", cmd_hello_start),
       SHELL_CMD(stop,  NULL, "Stop MAC-DECT app",       cmd_hello_stop),
       SHELL_CMD(mes,   NULL, "Send MAC text: hdect mac mes <text>", cmd_mac_mes),
       SHELL_SUBCMD_SET_END
   );

Ping utility
------------

.. code-block:: text

   hdect ping start [count]   # Start ping client
   hdect ping stop            # Stop ping client/server

Registered as:

.. code-block:: c

   SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_ping,
       SHELL_CMD(start,  NULL, "Start ping client: hdect ping start [count]", cmd_ping_start),
       SHELL_CMD(stop,   NULL, "Stop ping client/server",                     cmd_ping_stop),
       SHELL_SUBCMD_SET_END
   );

Performance metrics
-------------------

.. code-block:: text

   hdect perf start        # Start perf measurement
   hdect perf stop         # Stop perf measurement
   hdect perf reset        # Reset perf metrics
   hdect perf show         # Show perf metrics

Registered as:

.. code-block:: c

   SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_perf,
       SHELL_CMD(start, NULL, "Start perf measurement", cmd_hdect_perf_start),
       SHELL_CMD(stop,  NULL, "Stop perf measurement",  cmd_hdect_perf_stop),
       SHELL_CMD(reset, NULL, "Reset perf metrics",      cmd_hdect_perf_reset),
       SHELL_CMD(show,  NULL, "Show perf metrics",       cmd_hdect_perf_show),
       SHELL_SUBCMD_SET_END
   );

Configuration
-------------

The configuration command group is available as:

.. code-block:: text

   hdect cfg ...

.. note::

   The exact ``cfg`` subcommands depend on your :c:var:`sub_hdect_cfg` definition.

Shell tree registration
=======================


Future Work
***********

MAC and Scheduler roadmap
=========================

The HS_DECT_2020 codebase is designed to evolve from baseline PHY usage into a research-grade MAC platform.
Planned extensions include:

* **Multi-UE MAC support**
  - UE discovery/registration
  - addressing and session context
  - per-UE queues and per-UE scheduling

* **Scheduling policies (hs_scheduler)**
  - static baseline (round robin / fixed windows)
  - adaptive policies (load-aware, QoS-aware)
  - fairness metrics and starvation prevention

* **Reliability mechanisms**
  - optional ACK/NACK (research extension)
  - retransmission strategies and loss modeling
  - latency/reliability trade-off experiments

* **Measurement integration (hs_perf)**
  - standardized KPI export (CSV/log)
  - latency/jitter distributions
  - energy/performance measurement hooks

* **Automation**
  - scripted experiments using shell commands (UART)
  - repeatable test scenarios for thesis/papers

* **ML-based resource allocation**
  - policy selection / scheduling adaptation
  - prediction of traffic and link quality
  - comparative evaluation vs fixed schedulers


In addition, it uses the following secure firmware component:

* :ref:`Trusted Firmware-M <ug_tfm>`
