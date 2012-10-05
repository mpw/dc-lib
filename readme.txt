
-------------------------------------------------------------
INTRODUCTION

DC is a framework for adding dataflow constraints to C/C++ programs. It includes rm (a reactive memory manager), dc2 (a library for managing dataflow constraints), as well as various test programs and benchmarks.

DC includes portions of the Leonardo Library [http://www.dis.uniroma1.it/~leonardo/ll.shtml].

-------------------------------------------------------------
DOCUMENTATION

DC is presented and extensively discussed in the following conference paper:

* C. Demetrescu, I. Finocchi and A. Ribichini: "Reactive Imperative Programming with Dataflow Constraints", in Proceedings of the 26th ACM International Conference on Object-Oriented Programming, Systems, Languages and Applications (OOPSLA 2011), pp. 407-426, recipient of an OOPSLA Distinguished Paper award, 2011 [http://dl.acm.org/citation.cfm?id=2048100]

and in the following technical report:

* C. Demetrescu, I. Finocchi and A. Ribichini: "Reactive Imperative Programming with Dataflow Constraints", arXiv:1104.2293 [cs.PL], 2011 [http://arxiv.org/abs/1104.2293].

A detailed API reference can be found at the following url: [http://www.dis.uniroma1.it/~demetres/dc/].

-------------------------------------------------------------
LICENSE

DC is distributed under the GNU Lesser General Public License, Version 2.1, February 1999. Please see included file dc-lib/LICENSE.txt.

DC includes portions of the Leonardo Library [http://www.dis.uniroma1.it/~leonardo/ll.shtml]. The Leonardo Library is distributed under the GNU Lesser General Public License, Version 2.1, February 1999. Please see included file dc-lib/ll-core/LICENSE.txt.

-------------------------------------------------------------
PLATFORM DETAILS

DC has been designed to run on IA-32 Linux platforms. 

DC requires the following libraries: glib-2.0, libelf, libdisasm. 

DC has been tested on Linux Mandriva 2010.1 with gcc 4.4.3 and Qt 4.6.

-------------------------------------------------------------
QUICK START

1) Running 'make' from dc-lib/rm/ should create the rm library.

2) Running 'make' from dc-lib/dc2/ should create the dc library.

3) Various benchmarks and test programs can be compiled and launched through the makefiles in dc-lib/rm/test/, dc-lib/dc2/examples/, dc-lib/dc2/test/debug/, dc-lib/dc2/test/perf/.

4) To compile test programs that use the Qt library (e.g., those in subfolders of dc-lib/dc2/test/qt/), Qt's command 'qmake' should be used, in order to generate the actual makefiles, before executing 'make'.

