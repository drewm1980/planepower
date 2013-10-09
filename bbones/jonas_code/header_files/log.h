/*
 * AUTHOR: Jonas Van Pelt
 */
#ifndef LOG_H_ 
#define LOG_H_

/********************************
 * GLOBALS
 * ******************************/
 
enum log_errCode {LOG_ERR_NONE=0,LOG_ERR_UNDEFINED,LOG_ERR_MOUNT_SD,LOG_ERR_OPEN_FILE,LOG_ERR_WRITE,LOG_ERR_CLOSE};
typedef enum log_errCode LOG_errCode;

static const char FILE_PATH_LISA_LOG[] = "/media/rootfs/data_lisa_log.txt";
static const char FILE_PATH_GROUND_LOG[] = "/media/rootfs/data_groundstation_log.txt";
static const char FILE_PATH_BONEPLANE_LOG[] = "/media/rootfs/data_boneplane_log.txt";

static const char FILE_PATH_PROGRAM_LOG[]="log/log.txt";
static const char FILE_PATH_PROGRAM_ERROR[]="log/error.txt";

static const char SD_CARD_MOUNT_LOCATION[] = "/media/rootfs/";
static const char SD_CARD_DEVICE_LOCATION[] = "/dev/mmcblk0p2";

/********************************
 * PROTOTYPES PUBLIC
 * ******************************/

//for initialization the log
extern LOG_errCode init_log(); //check whether the sd card is present or not and tries to mount it

//for logging data coming from lisa
extern LOG_errCode open_data_lisa_log();
extern LOG_errCode write_data_lisa_log(char *data,int length);
extern LOG_errCode close_data_lisa_log();

//for logging data coming from groundstation
extern LOG_errCode open_data_groundstation_log();
extern LOG_errCode write_data_groundstation_log(char *data,int length);
extern LOG_errCode close_data_groundstation_log();

//for logging the main program
extern LOG_errCode log_write(char *file_name,char *message);
extern LOG_errCode error_write(char *file_name,char *message);

void LOG_err_handler(LOG_errCode err,void (*write_error_ptr)(char *,char *,int)); 

#endif /*LOG_H_*/
