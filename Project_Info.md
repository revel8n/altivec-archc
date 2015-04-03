# Altivec-ArchC. #

[SystemC](http://www.systemc.org) is a widely used system description language; SystemC is set of library routines and macros implemented in C++, which makes it possible to simulate concurrent processes, described by ordinary C++ syntax.

[ArchC](http://www.archc.org) is an architecture description language that [generates SystemC code](http://archc.sourceforge.net/index.php%3Fmodule=pagemaster&PAGE_user_op=view_page&PAGE_id=7&MMN_position=27:16.html).

Our project extends an existing ArchC model, [the PowerPC one](http://archc.sourceforge.net/index.php%3Fmodule=pagemaster&PAGE_user_op=view_page&PAGE_id=8&MMN_position=17:17.html), to include the Altivec instructions.

Altivec instructions are specified in the chapter 5 of [PowerIsa specification](http://altivec-archc.googlecode.com/files/PowerISA_Public.pdf).

# Getting. #

As of this, code is only available via svn.

svn co http://altivec-archc.googlecode.com/svn/trunk/

# Futher Reading. #

  * http://en.wikipedia.org/wiki/SystemC

  * http://www.lsc.ic.unicamp.br/publications/ARBAAB05.pdf - The ArchC Architecture Description Language