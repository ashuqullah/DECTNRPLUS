.. _dect_shell_application:

nRF91x1: DECT NR+ Shell
###########################

.. contents::
   :local:
   :depth: 2

The DECT NR+ physical layer (PHY) Shell (DeSh) sample application demonstrates how to
set up a DECT NR+ application with the DECT PHY firmware and enables testing of various
modem features.  

HS_DECT_SHELL builds upon this foundation by preserving the original PHY-level
functionality while extending the sample into an extensible experimentation and
development framework. The project is intended to evolve beyond isolated PHY tests
toward coordinated MAC-level procedures, group-based scheduling, and reproducible
experimentation workflows, without altering existing command behavior.

.. important::

   The sample showcases the use of the :ref:`nrf_modem_dect_phy` interface of the :ref:`nrfxlib:nrf_modem`.

Requirements
************

The sample supports the following development kits and requires at least two kits:

.. table-from-sample-yaml::

.. include:: /includes/tfm.txt

Project Overview
************

HS_DECT_SHELL is an evolutionary extension of the DECT NR+ physical layer (PHY) Shell
(DeSh) sample application provided in the nRF Connect SDK. The original DeSh sample
demonstrates how to set up a DECT NR+ application using the DECT PHY firmware and
provides interactive commands for testing various modem and PHY features.

Building on this foundation, HS_DECT_SHELL preserves all existing functionality and
behavior while introducing optional, backward-compatible extensions aimed at enabling
more structured experimentation and long-term project evolution. In particular, the
project focuses on adding higher-level coordination mechanisms on top of the existing
PHY capabilities, without modifying or replacing the underlying DECT PHY interfaces.

The primary goal of HS_DECT_SHELL is to extend the sample beyond isolated PHY-level
tests toward coordinated MAC-level procedures, group-based scheduling of operations,
and reproducible experimentation workflows. These additions allow complex multi-step
procedures—such as discovery, access, and association—to be executed in a deterministic
and observable manner.

HS_DECT_SHELL is intended for experimentation, research, and prototyping. It does not
aim to implement a full DECT MAC or a production-ready protocol stack. All extensions
are designed to be additive, non-intrusive, and compatible with upstream DeSh usage,
ensuring that existing commands and workflows remain unchanged.

Documentation Structure
-----------------------

The documentation in this repository is organized into multiple ``.rst`` files that are
included from the main ``README.rst``. This structure preserves the original DECT NR+
Shell documentation while allowing HS_DECT_SHELL-specific extensions to be documented
in a clear and modular way.

- ``README.rst`` (main entry point)
  Serves as the primary documentation entry point. It provides the project overview,
  scope, and consolidates the documentation by including the files listed below.

- ``dect_shell.rst`` (original DeSh documentation)
  Contains the original documentation of the DECT NR+ physical layer (PHY) Shell (DeSh)
  sample application. This file documents the existing commands, features, and usage
  of the upstream DeSh implementation and is kept unchanged to preserve reference
  behavior and compatibility.

- ``hs_dect_doc.rst`` (HS_DECT_SHELL overview)
  Describes the HS_DECT_SHELL project, including motivation, scope, design principles,
  and the relationship to the upstream DeSh sample. This file outlines the long-term
  evolution vision without altering the original functionality.

- ``hs_dect_group.rst`` (group scheduling extensions)
  Documents the group-based scheduling extensions introduced in HS_DECT_SHELL, focusing
  on MAC-level procedure orchestration, command structure, and the additive nature of
  the enhancements.
.. include:: dect_shell.rst
.. include:: hs_dect_doc.rst
.. include:: hs_dect_group.rst


DECT SHELL  Documentation 
************
 Original documentation of the DECT NR+ PHY Shell (DeSh) sample application.`HS DECT Documentation <dect_shel.rst>`_.

HS DECT Documentation 
************
Project-level documentation describing HS_DECT_SHELL extensions and evolution. `HS DECT Documentation <hs_dect_doc.rst>`_.



HS DECT group scheduling
************
Description of group-based scheduling and MAC-level orchestration features.`HS DECT group scheduling <hs_dect_group.rst>`_.


