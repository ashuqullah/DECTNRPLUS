HS_DECT_SHELL Documentation
==========================

Overview
--------

HS_DECT_SHELL is an extension of the DECT NR+ physical layer (PHY) Shell (DeSh) sample
application. It builds upon the original DECT PHY test framework provided in the
nRF Connect SDK, preserving existing functionality while introducing additional,
optional capabilities aimed at long-term project evolution.

The project focuses on extending DeSh beyond isolated PHY-level tests toward more
structured, coordinated, and reproducible experimentation.

---

Project Motivation
------------------

The original DeSh application provides a rich set of commands for testing DECT NR+
PHY and modem features. However, more complex use cases—such as multi-step MAC
procedures, coordinated scheduling, and repeatable experiments—require additional
orchestration logic.

HS_DECT_SHELL addresses these gaps by introducing higher-level abstractions on top
of existing functionality, without modifying or replacing the underlying PHY
interfaces or command behavior.

---

Scope and Design Principles
---------------------------

HS_DECT_SHELL follows these core principles:

- Backward compatibility with the original DeSh sample
- Additive and non-intrusive extensions
- Clear separation between PHY-level functionality and higher-layer coordination
- Suitability for experimentation, research, and prototyping

The project does not aim to implement a full DECT MAC or production-ready protocol
stack.

---

Key Extensions
--------------

The following extensions are introduced incrementally:

- Group-based scheduling of DECT procedures
- Orchestration of multi-step MAC-level workflows (for example, discovery and association)
- Improved structure and observability for complex test scenarios
- Support for reproducible and automated experimentation

All extensions are optional and coexist with existing DeSh commands.

---

Project Status and Evolution
----------------------------

HS_DECT_SHELL is an evolving project. Initial contributions focus on introducing
group scheduling and MAC procedure coordination. Future work may expand these
capabilities while maintaining compatibility with upstream DeSh developments.

Community feedback and incremental contributions are encouraged to guide the
project’s evolution.

---

Relationship to Upstream DeSh
-----------------------------

HS_DECT_SHELL is based on the DECT NR+ PHY Shell (DeSh) sample and does not replace it.
Where possible, changes are designed to be reusable or upstream-friendly, and the
project may serve as a reference for future enhancements to the original sample.
