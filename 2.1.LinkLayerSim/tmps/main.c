/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <stdio.h>

// #include "RSPkt.h"

// #include "RData.h"

// #include "RIPop.h"

// #include "RUdp.h"

// #include "RTcp.h"

// #include "RDstPkt.h"

#include "RTUp.h"

// #include "RSrvUdp.h"

// #include "RStr.h"

// #include "RMap.h"

int
main(int argc, char *argv[])
{
	// rsPkt_whitetest(NULL);

	// rData_whitetest();

	// rIpop_whitetest(RIP_DEFAULT_SUBNET_CHECK);

	// rUdp_whitetest();

	// rTcp_whitetest();

	// rDst_whitetest();

	rTUp_whitetest(argc, argv);

	// rSrvUdp_whitetest();

	// RStr_whitetest();

	// rMap_whitetest();

	return 0;
}