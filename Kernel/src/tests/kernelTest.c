#include "KernelTest.h"


int kernelTest(int flag){
	CU_initialize_registry();
	CU_basic_set_mode(CU_BRM_VERBOSE);

	CU_pSuite processTest = CU_add_suite("process",initializeProcessQueuesAndLists,destroyProcessQueuesAndLists);
	CU_add_test(processTest, "PIDFind", testPIDFind);
	/*CU_add_test(processTest, "newProcess",testNewProcess);
	CU_add_test(processTest, "readyProcessMultiprogOK",testReadyProcessMultiprogOK);
	CU_add_test(processTest, "readyProcessMultiprogNotOK",testReadyProcessMultiprogNotOK);*/

	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
