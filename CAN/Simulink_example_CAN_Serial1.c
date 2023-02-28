#include "Simulink_example_CAN_Serial1.h"
#include "Simulink_example_CAN_Serial1_private.h"

/* Block signals (default storage) */
B_Simulink_example_CAN_Serial_T Simulink_example_CAN_Serial1_B;

/* Block states (default storage) */
DW_Simulink_example_CAN_Seria_T Simulink_example_CAN_Serial1_DW;

/*
* Set which subrates need to run this base step (base rate always runs).
* This function must be called prior to calling the model step function
* in order to "remember" which rates need to run this base step.  The
* buffering of events allows for overlapping preemption.
*/
void Simulink_example_CAN_Serial1_SetEventsForThisBaseStep(boolean_T *eventFlags)
{
  /* Task runs when its counter is zero, computed via rtmStepTask macro */
  eventFlags[1] = ((boolean_T)rtmStepTask(Simulink_example_CAN_Serial1_M, 1));
  eventFlags[2] = ((boolean_T)rtmStepTask(Simulink_example_CAN_Serial1_M, 2));
  eventFlags[3] = ((boolean_T)rtmStepTask(Simulink_example_CAN_Serial1_M, 3));
}

/*
*   This function updates active task flag for each subrate
* and rate transition flags for tasks that exchange data.
* The function assumes rate-monotonic multitasking scheduler.
* The function must be called at model base rate so that
* the generated code self-manages all its subrates and rate
* transition flags.
*/
static void rate_monotonic_scheduler(void)
{
/* Compute which subrates run during the next base time step.  Subrates
* are an integer multiple of the base rate counter.  Therefore, the subtask
* counter is reset when it reaches its limit (zero means run).
*/
  (Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[1])++;
  if ((Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[1]) > 99) {/* Sample time: [0.1s, 0.0s] */
    Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[1] = 0;
  }
  (Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[2])++;

  if ((Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[2]) > 499) {/* Sample time: [0.5s, 0.0s] */
    Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[2] = 0;
  }
  (Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[3])++;

  if ((Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[3]) > 999) {/* Sample time: [1.0s, 0.0s] */
  Simulink_example_CAN_Serial1_M->Timing.TaskCounters.TID[3] = 0;
  }
}

/* Model step function for TID0 */
81	void Simulink_example_CAN_Serial1_step0(void) /* Sample time: [0.001s, 0.0s] */
82	{
83	  MW_SPI_FirstBitTransfer_Type MsbFirstTransferLoc;
84	  MW_SPI_Mode_type ClockModeValue;
85	  uint16_T rtb_DutyCycleBuck;
86
87	  {                                    /* Sample time: [0.001s, 0.0s] */
88	    rate_monotonic_scheduler();
89	  }
90
91	  /* ManualSwitch: '<Root>/Manual Switch1' incorporates:
92	   *  Constant: '<Root>/ 1'
93	   *  Constant: '<Root>/src2'
94	   */
95	  if (Simulink_example_CAN_Serial1_P.ManualSwitch1_CurrentSetting == 1U) {
96	    rtb_DutyCycleBuck = Simulink_example_CAN_Serial1_P.src2_Value;
97	  } else {
98	    rtb_DutyCycleBuck = Simulink_example_CAN_Serial1_P.u_Value;
99	  }
100
101	  /* End of ManualSwitch: '<Root>/Manual Switch1' */
102
103	  /* S-Function (c2802xpwm): '<Root>/ePWM' */
104
105	  /*-- Update CMPA value for ePWM1 --*/
106	  {
107	    EPwm1Regs.CMPA.bit.CMPA = (uint16_T)((uint32_T)EPwm1Regs.TBPRD *
108	      rtb_DutyCycleBuck * 0.01);
109	  }
110
111	  /* MATLABSystem: '<Root>/SPI Transmit' incorporates:
112	   *  Constant: '<Root>/ 2'
113	   */
114	  MW_SPI_SetSlaveSelect(Simulink_example_CAN_Serial1_DW.obj_p.MW_SPI_HANDLE, 0U,
115	                        true);
116	  ClockModeValue = MW_SPI_MODE_0;
117	  MsbFirstTransferLoc = MW_SPI_MOST_SIGNIFICANT_BIT_FIRST;
118	  rtb_DutyCycleBuck = MW_SPI_SetFormat
119	    (Simulink_example_CAN_Serial1_DW.obj_p.MW_SPI_HANDLE, 8U, ClockModeValue,
120	     MsbFirstTransferLoc);
121	  if (rtb_DutyCycleBuck == 0U) {
122	    MW_SPI_Write_16bits(Simulink_example_CAN_Serial1_DW.obj_p.MW_SPI_HANDLE,
123	                        &Simulink_example_CAN_Serial1_P.u_Value_d, 1UL, 0U);
124	  }
125
126	  /* End of MATLABSystem: '<Root>/SPI Transmit' */
127
128	  /* S-Function (c28xsci_tx): '<Root>/SCI Transmit' incorporates:
129	   *  Constant: '<Root>/ 3'
130	   */
131	  {
132	    /* Send additional data header */
133	    {
134	      char *String = "S";
135	      scia_xmit(String, 1, 1);
136	    }
137
138	    scia_xmit((char*)&Simulink_example_CAN_Serial1_P.u_Value_n, 2, 2);
139
140	    /* Send additional data terminator */
141	    {
142	      char *String = "E";
143	      scia_xmit(String, 1, 1);
144	    }
145	  }
146
147	  /* S-Function (c280xcanxmt): '<Root>/eCAN Transmit' incorporates:
148	   *  Constant: '<Root>/ 4'
149	   */
150	  {
151	    tCANMsgObject sTXCANMessage;
152	    unsigned char ucTXMsgData[2];
153	    ucTXMsgData[0] = (Simulink_example_CAN_Serial1_P.u_Value_e & 0xFF);
154	    ucTXMsgData[1] = (Simulink_example_CAN_Serial1_P.u_Value_e >> 8);
155	    sTXCANMessage.ui32MsgIDMask = 0;   // no mask needed for TX
156	    sTXCANMessage.ui32MsgLen = 2;      // size of message
157	    sTXCANMessage.ui32MsgID = 455;     // CAN message ID - use 1
158	    sTXCANMessage.pucMsgData = ucTXMsgData;// ptr to message content
159	    sTXCANMessage.ui32Flags = MSG_OBJ_NO_FLAGS;
160	    CANMessageSet(CANB_BASE, 2, &sTXCANMessage, MSG_OBJ_TYPE_TX);
161	  }
162
163	  /* S-Function (c280xi2c_tx): '<Root>/I2C Transmit' incorporates:
164	   *  Constant: '<Root>/ 5'
165	   */
166	  {
167	    int unsigned tx_loop= 0;
168	    while (I2caRegs.I2CFFTX.bit.TXFFST!=0 && tx_loop<10000 )
169	      tx_loop++;
170	    if (tx_loop!=10000) {
171	      I2caRegs.I2CSAR.bit.SAR = 80;    /* Set slave address*/
172	      I2caRegs.I2CCNT= 2;              /* Set data length */
173
174	      /* mode:1 (1:master 0:slave)  Addressing mode:0 (1:10-bit 0:7-bit)
175	         free data mode:0 (1:enbaled 0:disabled) digital loopback mode:0 (1:enabled 0:disabled)
176	         bit count:0 (0:8bit) stop condition:0 (1:enabled 0: disabled)*/
177	      I2caRegs.I2CMDR.all = 26144;
178	      tx_loop= 0;
179	      while (I2caRegs.I2CFFTX.bit.TXFFST>14 && tx_loop<10000)
180	        tx_loop++;
181	      if (tx_loop!=10000) {
182	        I2caRegs.I2CDXR.bit.DATA = (uint8_T)
183	          (Simulink_example_CAN_Serial1_P.u_Value_n1&0xFF);
184	        I2caRegs.I2CDXR.bit.DATA = (uint8_T)
185	          ((Simulink_example_CAN_Serial1_P.u_Value_n1>>8&0xFF));
186	      }
187	    }
188	  }
189
190	  /* S-Function (c2802xadc): '<Root>/ADC' */
191	  {
192	    /*  Internal Reference Voltage : Fixed scale 0 to 3.3 V range.  */
193	    /*  External Reference Voltage : Allowable ranges of VREFHI(ADCINA0) = 3.3 and VREFLO(tied to ground) = 0  */
194	    Simulink_example_CAN_Serial1_B.ADC = (AdcaResultRegs.ADCRESULT0);
195	  }
196
197	  /* S-Function (c280xi2c_rx): '<Root>/I2C Receive' */
198	  {
199	    int rx_loop= 0;
200	    int8_T rx_output= 0;
201	    I2caRegs.I2CSAR.bit.SAR = 80;      /* Set slave address*/
202	    I2caRegs.I2CCNT= 1;                /* Set data length */
203
204	    /* mode:1 (1:master 0:slave)  Addressing mode:0 (1:10-bit 0:7-bit)
	       free data mode:0 (1:enbaled 0:disabled) digital loopback mode:0 (1:enabled 0:disabled)
	       bit count:0 (0:8bit) NACK mode:0 (1:enabled 0: disabled) stop condition:0 (1:enabled 0: disabled)*/
	    I2caRegs.I2CMDR.all = 25632;
      rx_loop= 0;
	    rx_output= 0;
	    while (I2caRegs.I2CFFRX.bit.RXFFST==0 && rx_loop<10000)
      rx_loop++;
	    if (rx_loop!=10000) {
      rx_output = I2caRegs.I2CDRR.bit.DATA;
      if (rx_output > 127) {
      Simulink_example_CAN_Serial1_B.I2CReceive = rx_output-256;
      } else {
        Simulink_example_CAN_Serial1_B.I2CReceive = rx_output;
      }
    }
  }
}
