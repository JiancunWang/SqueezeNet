#include "weights.hpp"

u32 frequency_set(int pl_clock, int divisor0, int divisor1) ;

XScuTimer Timer;
XCompute compute;
XCompute_Config config;

u32* DRAM = (u32*)DRAM_ADDR;


int main()
{
	int status, i, j;
	XCompute_Config *CfgPtr;

	//timer initialization
	XScuTimer_Config *TMRConfigPtr;
	TMRConfigPtr = XScuTimer_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	XScuTimer_CfgInitialize(&Timer, TMRConfigPtr,TMRConfigPtr->BaseAddr);
	XScuTimer_SelfTest(&Timer);

    printf("Hello compute\n");

    CfgPtr = XCompute_LookupConfig(XPAR_COMPUTE_0_DEVICE_ID);
    if(!CfgPtr){
		printf("Error looking for AXI DMA config\n");
		return XST_FAILURE;
    }

    status = XCompute_CfgInitialize(&compute,CfgPtr);
    if(status != XST_SUCCESS){
		print("Error initializing DMA\n\r");
		return XST_FAILURE;
    }

    printf("before creating network\n");

    DRAM = (u32*)weights;
    network_t* network = create_network(NUM_LAYERS, TOT_NUM_WEIGHTS);

    	u32 compute_value = 0;
    	u32 load_value = 0;

    	XScuTimer_LoadTimer(&Timer, TIMER_LOAD_VALUE);
    				XScuTimer_Start(&Timer);
    for (j = 0; j < 1000; j++) {
    	printf("iter: %d...\n", j);
		for (i = 0; i < network->num_layers; i++) {
			load_layer(network->layers[i]);
			XCompute_Start(&compute);

			while (!XCompute_IsDone(&compute)){}
		}
    }
    printf("all iters ran!\n");
    printf("Time %f\n", 0.002*(0xFFFFFFFF - XScuTimer_GetCounterValue(&Timer))/666.66);

	int num_layers = network->num_layers;
	layer_t final_layer = network->layers[num_layers-1];
	int output_offset = final_layer.output_offset;
	int output_size = final_layer.chan_out;

	data_t *B = (data_t *)&DRAM[output_offset];

	data_t denom = 0;
	for (i = 0; i < output_size; i++) {
		denom += exp(B[i]);
	}

	for (i = 0; i < output_size; i++) {
		B[i] = exp(B[i])/denom;
	}

    print("Bye compute\n\r");
    return 0;
}