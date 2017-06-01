#include "KernelTest.h"


int kernelTest(int flag){
	CU_initialize_registry();
	CU_basic_set_mode(CU_BRM_VERBOSE);

	CU_pSuite processTest = CU_add_suite("process",initializeProcessQueuesAndLists,destroyProcessQueuesAndLists);
	CU_add_test(processTest, "PIDFind", testPIDFind);
	CU_add_test(processTest, "modifyProcessState", testModifyProcessState);
	CU_add_test(processTest, "newProcess",testNewProcess);
	CU_add_test(processTest, "readyProcessMultiprogOK",testReadyProcessMultiprogOK);
	CU_add_test(processTest, "readyProcessMultiprogNotOK",testReadyProcessMultiprogNotOK);
	CU_add_test(processTest, "executeProcessCPUOK",testExecuteProcessCPUOk);
	CU_add_test(processTest, "executeProcessCPUNotOK",testExecuteProcessCPUNotOk);
	CU_add_test(processTest, "CPUReturnsProcessToReady", testCPUReturnsProcessToReady);
	CU_add_test(processTest, "CPUReturnsProcessToBlocked", testCPUReturnsProcessToBlocked);


	CU_pSuite heapTest = CU_add_suite("heap",NULL,NULL);
	CU_add_test(heapTest, "serializationMemReqTest", testSerializationMemoryRequest);
	CU_add_test(heapTest, "pageToStoreTestPageAvailable", pageToStoreTestPageAvailable);
	CU_add_test(heapTest, "pageToStoreTestPageUnavailable", pageToStoreTestPageUnavailable);


	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
