// *****************************************************************************
// To Compile without cmake :
// g++ Main_SocketSHM_B.cc SocketSHMPosix.cc SemaphorePosix.cc SocketSHMSysV.cc SHMSysV.cc SemaphoreSysV.cc -o Processus_B -lrt
// *****************************************************************************
// For SysV use :
// Before Use : --> #ipcs 
// You can see all the semaphore and the shared memory in use
//  --> #ipcrm -m id_memory to erase a memory segment
//  --> #ipcrm -s id_semaphore to erase a semaphore

// Systems includes
#include <limits> 

// Specifics includes
#include "SocketSHMPosix.hh"
#include "SocketSHMSysV.hh"
#include "SharedStruct.hh"

#define NAME_A_TO_B "/foo1234"  // Name SHM to broadcast from A to B 
#define NAME_B_TO_A "/foo5678" // Name SHM to broadcast from B to A 

#define KEY_A_TO_B 0x12345 // Key SHM to broadcast from A to B 
#define KEY_B_TO_A 0x6789 // Key SHM to broadcast from B to A 

#define NAME_FULL_AB "/Sem_full_AB" // Name Full Semaphore to synchronise broadcast from A to B (Useful for Posix Semaphore)
#define NAME_EMPTY_AB "/Sem_empty_AB" // Name Empty Semaphore to synchronise broadcast from A to B (Useful for Posix Semaphore)
#define NAME_FULL_BA "/Sem_full_BA" // Name Full Semaphore to synchronise broadcast from B to A (Useful for Posix Semaphore)
#define NAME_EMPTY_BA "/Sem_empty_BA" // Name Empty Semaphore to synchronise broadcast from B to A (Useful for Posix Semaphore)

#define KEY_FULL_AB 0x12 // // Key Full Semaphore to synchronise broadcast from A to B (Useful for SysV Semaphore)
#define KEY_EMPTY_AB 0x34 // Key Empty Semaphore to synchronise broadcast from A to B (Useful for SysV Semaphore)
#define KEY_FULL_BA 0x56 // Key Full Semaphore to synchronise broadcast from B to A (Useful for SysV Semaphore)
#define KEY_EMPTY_BA 0x78 // Key Empty Semaphore to synchronise broadcast from B to A (Useful for SysV Semaphore)


/****************************************/
/******** PROGRAMME PRINCIPAL ***********/
/****************************************/

int main(){

// ************
// DECLARATIONS  
// ************ 

int i = 1 ; // variable quelconque qui me sers pour un compteur plus bas...
shared_struct Data_Read_B ; // Data lue dans la socketSHM.
shared_struct Data_Write_B ; // Data ecrite dans la socketSHM.
std::string command;

int size ;
size = (int) sizeof(shared_struct) ;

// ************
//    CODE
// ************

// Posix Socket SHM
SocketSHMPosix Socket_Posix_AB(NAME_A_TO_B, 
			       NAME_B_TO_A, 
                               false,
                               true,
                               size, 
                               size, 
                               NAME_FULL_AB, 
                	       NAME_EMPTY_AB, 
                               NAME_FULL_BA, 
                               NAME_EMPTY_BA) ; 

// SystemV Socket SHM
SocketSHMSysV Socket_SysV_AB(NAME_A_TO_B, 
			     NAME_B_TO_A, 
                             KEY_A_TO_B, 
			     KEY_B_TO_A, 
                             false,
                             true,
                             size, 
                             size, 
                             KEY_FULL_AB, 
                	     KEY_EMPTY_AB, 
                             KEY_FULL_BA, 
                             KEY_EMPTY_BA) ; 

// Wainting for User Command n1
std::cout << "*******************************************************" << std::endl ;
std::cout << "*********** END OF INITIALIZATION PHASE 1 : ***********" << std::endl ;
std::cout << "*** Click \"Posix\" to Open Posix Socket SHM **********" << std::endl ;
std::cout << "*** Click \"SysV\" to Open Systeme V Socket SHM *******" << std::endl ;
std::cout << "*******************************************************" << std::endl ;
std::cin >> command;
std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );

// Open the Socket
if (command=="Posix") Socket_Posix_AB.Open() ;
if (command=="SysV") Socket_SysV_AB.Open() ;

// Wainting for User Command n2
std::cout << "******************************************************" << std::endl ;
std::cout << "******* END OF INITIALIZATION PHASE 2 : **************" << std::endl ;
std::cout << "*** Click \"ENTER\" to Run Inter Process Exchange ****" << std::endl ;
std::cout << "******************************************************" << std::endl ;
std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );

 while(i<1000) {
    std::cout << "                                                    " << std ::endl ;
    std::cout << " ***********  COMPUTING PHASE n°= " << i << "*************" << std ::endl ;

    /**************************************/
    /************* RECEIVING **************/
    /**************************************/
    // Receive from A
    if (command=="Posix") Socket_Posix_AB.Receive(&Data_Read_B) ;
    if (command=="SysV") Socket_SysV_AB.Receive(&Data_Read_B) ;

    /**************************************/
    /************* COMPUTE ****************/
    /**************************************/
   // Print Receiving Data
   std::cout << " ****** RECEIVE ******" << std ::endl ;
   std::cout << "Processus B || DataRead.Header = " << Data_Read_B.Header << std ::endl ;
   std::cout << " Processus B || DataRead.Body = " << Data_Read_B.Body << std ::endl ;
   std::cout << " ************************** " <<  std ::endl ;

    // Product a new Data
    Data_Write_B.Header = Data_Read_B.Header + 1 ;
    Data_Write_B.Body = Data_Read_B.Body + 0.1 ;

    // Print Sending Data
    std::cout << " ******** SEND ********" << std ::endl ;
    std::cout << "Processus B || DataWrite.Header = " << Data_Write_B.Header << std ::endl ;
    std::cout << " Processus B || DataWrite.Body = " << Data_Write_B.Body << std ::endl ;
    std::cout << " ************************ " <<  std ::endl ;
    
    /**************************************/
    /************* SENDING ****************/
    /**************************************/
   // Send to A
    if (command=="Posix") Socket_Posix_AB.Send(&Data_Write_B) ;
    if (command=="SysV") Socket_SysV_AB.Send(&Data_Write_B) ;

    // On incremente le compteur
    i++ ;

    } // Fin de la boucle while

// Waiting the two process are out of while boucle
sleep(1) ; 

// Close the socket
if (command=="Posix") Socket_Posix_AB.Close() ; 
if (command=="SysV") Socket_SysV_AB.Close() ;

} // Fin Programme Principal
