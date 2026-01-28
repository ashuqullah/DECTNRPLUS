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

   Your project uses the command name ``hdect`` (instead of ``dout``).

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

The full command tree is assembled as follows:

.. code-block:: c

   SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect,
       SHELL_CMD(led,  &sub_hdect_led,  "LED control",       NULL),
       SHELL_CMD(mac,  &sub_hdect_mac,  "MAC-DECT control",  NULL),
       SHELL_CMD(ping, &sub_hdect_ping, "DECT ping utility", NULL),
       SHELL_CMD(cfg,  &sub_hdect_cfg,  "HS-DECT config",    NULL),
       SHELL_CMD(perf, &sub_hdect_perf, "Performance metrics", NULL),
       SHELL_SUBCMD_SET_END
   );

LED initialization at boot
==========================

LEDs are initialized at boot time using:

.. code-block:: c

   static int hs_shell_init(const struct device *dev)
   {
       ARG_UNUSED(dev);
       return leds_init();
   }

   SYS_INIT(hs_shell_init, APPLICATION, 90);


Screenshots
***********

Repository structure
====================

.. figure:: doc/img/repo_tree.png
   :alt: HS_DECT_2020 repository tree
   :align: center
   :width: 90%

   HS_DECT_2020 repository structure showing the main modules under :file:`src/`.

Shell usage
===========

.. figure:: doc/img/hdect_shell_help.png
   :alt: hdect shell command help
   :align: center
   :width: 90%

   Example output of ``hdect`` help showing available subcommands.

Perf and ping examples
======================

.. figure:: doc/img/perf_show.png
   :alt: hdect perf show output
   :align: center
   :width: 90%

   Example output from ``hdect perf show``.

.. figure:: doc/img/ping_start.png
   :alt: hdect ping start output
   :align: center
   :width: 90%

   Example output from ``hdect ping start``.


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
