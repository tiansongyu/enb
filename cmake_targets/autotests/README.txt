OAI Test PLAN
#UNDER CONSTRUCTION. Not correct at the moment

Obj.#   Case#   Test#	Description

01                      pre-commit test case
01      01              Build OAI 
01      01      00      Check Makefiles and 
01      01      01      Build oaisim Rel8
01      01      02      Build oaisim Rel8 + network device driver(nasmesh_fix)	
01      01      03      Build lte-softmode Rel8 	
01      01      04      Build dlsim  Rel8
01      01      05      Build ulsim  Rel8
01      01      06      Build oaisim Rel10
01      01      07      Build oaisim Rel8 with cellular RRC for eNB
01      01      08      Build oaisim Rel8 with cellular RRC for UE
01      01      09      "commented test in targets/TEST/OAI/case01.py"
01      01      10      "commented test in targets/TEST/OAI/case01.py" 
01      01      11      "commented test in targets/TEST/OAI/case01.py" 
01      01      12      "commented test in targets/TEST/OAI/case01.py" 
01      01      13      Build network device driver(oai_nw_drv type ethernet)
01      01      14      Build oaisim Rel8 with RRC lite (new cellular+ITTI) + RAL (802.21) 
01      01      15      Build oaisim Rel10 with RRC lite (new cellular+ITTI) + RAL (802.21) 

01      02              Run OAI Rel8, and check the operation
01      02      00      Run OAI Rel8, and search for segmentation fault or exit
01      02      01      Run OAI Rel8, and search for execution errors
01      02      02      Run OAI Rel8 in abstraction mode and check that RRC proc is finished completely for the configured number of eNB and UE
01      02      03      Run OAI Rel8 in abstraction mode, send ping from from one eNB to each UE, and check that there is no packet losses
01      02      04      Run OAI Rel8 with full PHY, and check that the RRC proc for eNBsxUEs
01      02      05      Run OAI Rel8 with full PHY in FDD mode, and check that the RRC proc for eNBsxUEs

01      03              Run OAI Rel10, and check the operation
01      03      00      Run OAI Rel10, and search for segmentation fault or exit
01      03      01      Run OAI Rel10, and search for execution errors
01      03      02      Run OAI Rel10 in abstraction mode, and check the RRC proc for eNBsxUEs	
01      03      03      Run OAI Rel10 in full phy mode, and check the RRC proc for eNBsxUEs
01      03      04      Run OAI Rel10 in full phy mode in FDD mode, and check the RRC proc for eNBsxUEs
01      03      05      Run OAI Rel10 with eMBMS enabled, and check the SIB13 and MCCH
01      03      06      Run OAI Rel10 with eMBMS enabled, and check the MTCH
01      03      07      Run OAI Rel10 with eMBMS enabled and FDD mode, and check the MTCH

02                      Functional test case

03                      Non-Functional test case

04                      Failure test case 
 
05                      Performance test case 

