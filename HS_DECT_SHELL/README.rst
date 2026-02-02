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

Overview
********

`HS_DECT_SHELL Documentation <hs_dect_doc.rst>`_.
===========================
`HS_DECT_SHELL Documentation <hs_dect_doc.rst>`_.


`HS DECT group scheduling <hs_dect_group.rst>`_.
========================
`HS DECT group scheduling <hs_dect_group.rst>`_.
