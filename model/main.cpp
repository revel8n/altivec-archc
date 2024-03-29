/******************************************************
 * This is the main file for the powerpc ArchC model  *
 * This file is automatically generated by ArchC      *
 * WITHOUT WARRANTY OF ANY KIND, either express       *
 * or implied.                                        *
 * For more information on ArchC, please visit:       *
 * http://www.archc.org                               *
 *                                                    *
 * The ArchC Team                                     *
 * Computer Systems Laboratory (LSC)                  *
 * IC-UNICAMP                                         *
 * http://www.lsc.ic.unicamp.br                       *
 ******************************************************/
 

const char *project_name="powerpc";
const char *project_file="powerpc.ac";
const char *archc_version="2.0beta3";
const char *archc_options="-abi ";

#include  <iostream>
#include  <systemc.h>
#include  "ac_stats_base.H"
#include  "powerpc.H"



int sc_main(int ac, char *av[])
{

  //!  ISA simulator
  powerpc powerpc_proc1("powerpc");

#ifdef AC_DEBUG
  ac_trace("powerpc_proc1.trace");
#endif 

  powerpc_proc1.init(ac, av);
  cerr << endl;

  sc_start();

  powerpc_proc1.PrintStat();
  cerr << endl;

#ifdef AC_STATS
  ac_stats_base::print_all_stats(std::cerr);
#endif 

#ifdef AC_DEBUG
  ac_close_trace();
#endif 

  return powerpc_proc1.ac_exit_status;
}
