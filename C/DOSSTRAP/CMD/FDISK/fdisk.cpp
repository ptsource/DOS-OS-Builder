/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#define NAME "FDISK"
#define SIZE_OF_MBR 445
#define EMULATED_CYLINDERS   784
#define EMULATED_HEADS       255
#define EMULATED_SECTORS      63
#include <bios.h>
#include <conio.h>
#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbr.h"
#define READ 2
#define TRUE 1
#define FALSE 0
#define UNCHANGED 20
#define UNUSED    99
#define THREETHREE 33
#define FOUR       4
#define FIVE       5
#define SIX        6
#define W95        7
#define W95B       72
#define W98        8
#define MEG 1048576
#define PRIMARY 1
#define EXTENDED 2
#define LOGICAL 3
#define SPECIAL 4
#define LAST 99
#define PERCENTAGE 1
#define STANDARD 0
#define TECHNICAL 1
#define BOLD 1
#define INTERNAL 0
#define EXTERNAL 1
#define MM   0x00               /* Main Menu                     */
#define CP   0x10             /* Create PDP or LDD             */
#define CPDP 0x11           /* Create Primary DOS Partition  */
#define CEDP 0x12           /* Create Extended DOS Partition */
#define CLDD 0x13           /* Create Logical DOS Drive      */
#define SAP  0x20             /* Set Active Partition          */
#define DP   0x30             /* Delete partition or LDD       */
#define DPDP 0x31           /* Delete Primary DOS Partition  */
#define DEDP 0x32           /* Delete Extended DOS Partition */
#define DLDD 0x33           /* Delete Logical DOS Drive      */
#define DNDP 0x34           /* Delete Non-DOS Partition      */
#define DPI  0x40             /* Display Partition Information */
#define CD   0x50             /* Change Drive                  */
#define EXIT 0x0f               /* Code to Exit from Program     */
#define YN     0
#define NUM    1
#define NUMP   2
#define ESC    3
#define ESCR   4
#define ESCE   5
#define ESCC   6
#define CHAR   7
#define NONE   8
#define ADD      0
#define SUBTRACT 1
#define MULTIPLY 2
#define DIVIDE   3
char filename[256];
char path[256];
char partition_label[16];
unsigned char sector_buffer[512];
unsigned char partition_lookup_table_buffer_short[256] [9];
unsigned char partition_lookup_table_buffer_long[256] [17];
int brief_partition_table[8] [27];
char drive_lettering_buffer[8] [27];
unsigned long computed_ending_cylinder;
unsigned long computed_partition_size;
unsigned char disk_address_packet[16];
void *lba_address_l=&disk_address_packet[12];
void *transfer_buffer_segment=&disk_address_packet[4];
void *transfer_buffer_offset=&disk_address_packet[6];
int integer1;
int integer2;
unsigned long sixty_four_answer_h;
unsigned long sixty_four_answer_l;
typedef struct debugging_table_structure {
    int all;
    int create_partition;
    int determine_free_space;
    int emulate_disk;
    int input_routine;
    int LBA;
    int path;
    int write;
} Debugging_Table;
typedef struct flags_structure {
    int display_name_description_copyright;
    int drive_number;
    int esc;
    int extended_int_13;
    int extended_options_flag;
    int fat32;
    int label;
    int monochrome;
    int maximum_drive_number;
    int more_than_one_drive;
    int partitions_have_changed;
    int partition_type_lookup_table;
    int reboot;
    int version;
    int use_ambr;
} Flags;
typedef struct partition_table_structure {
    unsigned long total_cylinders;
    unsigned long total_heads;
    unsigned long total_sectors;
    unsigned long total_logical_sectors;
    unsigned long total_hard_disk_size_in_logical_sectors;
    unsigned long total_hard_disk_size_in_MB;
    int partition_values_changed;
    int primary_partition_exists;
    int active_status[4];
    int primary_partition_numeric_type[4];
    char primary_partition_volume_label[4] [13];
    long primary_partition_starting_cylinder[4];
    long primary_partition_starting_head[4];
    long primary_partition_starting_sector[4];
    long primary_partition_ending_cylinder[4];
    long primary_partition_ending_head[4];
    long primary_partition_ending_sector[4];
    unsigned long primary_partition_relative_sectors[4];
    unsigned long primary_partition_number_of_sectors[4];
    long primary_partition_size_in_MB[4];
    unsigned long primary_partition_largest_free_space;
    long pp_largest_free_space_starting_cylinder;
    long pp_largest_free_space_starting_head;
    long pp_largest_free_space_starting_sector;
    long pp_largest_free_space_ending_cylinder;
    int primary_partition_physical_order[4];
    int extended_partition_exists;
    int number_of_extended_partition;
    long extended_partition_size_in_MB;
    unsigned long extended_partition_number_of_sectors;
    unsigned long extended_partition_largest_free_space;
    long logical_drive_largest_free_space_location;
    long logical_drive_free_space_starting_cylinder;
    long logical_drive_free_space_ending_cylinder;
    int number_of_logical_drives;
    int logical_drive_numeric_type[23];
    char logical_drive_volume_label[23] [13];
    long logical_drive_starting_cylinder[23];
    long logical_drive_starting_head[23];
    long logical_drive_starting_sector[23];
    long logical_drive_ending_cylinder[23];
    long logical_drive_ending_head[23];
    long logical_drive_ending_sector[23];
    unsigned long logical_drive_relative_sectors[23];
    unsigned long logical_drive_number_of_sectors[23];
    long logical_drive_size_in_MB[23];
    int next_extended_exists[23];
    int next_extended_numeric_type[23];
    long next_extended_starting_cylinder[23];
    long next_extended_starting_head[23];
    long next_extended_starting_sector[23];
    long next_extended_ending_cylinder[23];
    long next_extended_ending_head[23];
    long next_extended_ending_sector[23];
    unsigned long next_extended_relative_sectors[23];
    unsigned long next_extended_number_of_sectors[23];
} Partition_Table;
Debugging_Table debug;
Flags flags;
Partition_Table partition_table[8];
int Create_DOS_Partition_Interface(int numeric_type, long size_in_MB);
int Create_Logical_Drive(int numeric_type, long size_in_MB);
int Create_Logical_Drive_Interface();
int Create_Primary_Partition(int numeric_type,long size_in_MB);
int Delete_Logical_Drive_Interface();
int Determine_Drive_Letters();
int More_Than_One_Hard_Disk();
int Read_Partition_Tables();
int Read_Physical_Sector(int drive, long cylinder, long head, long sector);
int Set_Active_Partition_Interface();
int Standard_Menu(int menu);
int Write_Partition_Tables();
int Write_Physical_Sector(int drive, long cylinder, long head, long sector);
unsigned long Combine_Cylinder_and_Sector(unsigned long cylinder, unsigned long sector);
unsigned long Decimal_Number(unsigned long hex1, unsigned long hex2, unsigned long hex3, unsigned long hex4);
unsigned long Extract_Cylinder(unsigned long hex1, unsigned long hex2);
unsigned long Extract_Sector(unsigned long hex1, unsigned long hex2);
unsigned long Input(int size_of_field,int x_position,int y_position,int type, int min_range, long max_range,int return_message,int default_value,long maxiumum_possible_percentage);
void Allocate_Memory_For_Buffers();
void Ask_User_About_FAT32_Support();
void Automatically_Partition_Hard_Drive();
void Calculate_Partition_Ending_Cylinder(long start_cylinder,long size);
void Check_For_INT13_Extensions();
void Clear_Active_Partition();
void Clear_Partition_Table();
void Clear_Partition_Table_Area_Of_Sector_Buffer();
void Clear_Screen();
void Clear_Sector_Buffer();
void Convert_Long_To_Integer(long number);
void Create_Alternate_MBR();
void Create_MBR();
void Create_MBR_If_Not_Present();
void Delete_Extended_DOS_Partition_Interface();
void Delete_Logical_Drive(int type);
void Delete_N_DOS_Partition_Interface();
void Delete_Primary_Partition(int partition_number);
void Delete_Primary_DOS_Partition_Interface();
void Determine_Color_Video_Support();
void Determine_Free_Space();
void Display_All_Drives();
void Display_CL_Partition_Table();
void Display_Help_Screen();
void Display_Information();
void Display_Label();
void Display_Extended_Partition_Information_SS();
void Display_Or_Modify_Partition_Information();
void Display_Partition_Information();
void Display_Primary_Partition_Information_SS();
void Dump_Partition_Information();
void Get_Partition_Information();
void Initialization();
void Interactive_User_Interface();
void Load_Brief_Partition_Table();
void Load_LBA_Into_DAP(unsigned long lba_address);
void Menu_Routine();
void Modify_Partition_Type(int partition_number,int type_number);
void Pause();
void Position_Cursor(int row,int column);
void Print_Centered(int y,char *text,int style);
void Process_Fdiskini_File();
void Reboot_PC();
void Remove_MBR();
void Save_MBR();
void Set_Active_Partition(int partition_number);
void Sixty_Four_Bit_Math(unsigned long sixty_four_ah, unsigned long sixty_four_al, unsigned long sixty_four_bh, unsigned long sixty_four_bl, int instruction);
void Ask_User_About_FAT32_Support() {
    Clear_Screen();
    Print_Centered(5,"FDISK is capable of using large disk support to allow you to         ",0);
    Print_Centered(6,"create partitions that are greater than 2,048 MB by using FAT32      ",0);
    Print_Centered(7,"partitions.  If you enable large disk support, any partitions or     ",0);
    Print_Centered(8,"logical drives greater than 512 MB will be created using FAT32.      ",0);
    Print_Centered(17,"Do you want to use large disk support (Y/N)....?    ",0);
    flags.fat32=Input(1,62,17,YN,0,0,NONE,1,0);
}
void Automatically_Partition_Hard_Drive() {
    int index=0;
    unsigned long maximum_partition_size_in_MB;
    Determine_Drive_Letters();
    do {
        if( (brief_partition_table[(flags.drive_number-128)] [index]>0) && (brief_partition_table[(flags.drive_number-128)][index]!=18) ) {
            printf("\nThe hard drive has already been partitioned...Operation Terminated.\n");
            exit(7);
        }
        index++;
    } while(index<4);
    Determine_Free_Space();
    Set_Active_Partition(Create_Primary_Partition(6,2048));
    Determine_Free_Space();
    if( ( ( (partition_table[(flags.drive_number-128)].primary_partition_largest_free_space+1) * (partition_table[(flags.drive_number-128)].total_heads+1) * partition_table[(flags.drive_number-128)].total_sectors)/2048)>3) {
        Create_Primary_Partition(5,9000);
        Determine_Free_Space();
        do {
            Create_Logical_Drive(6,2048);
            Determine_Free_Space();
            maximum_partition_size_in_MB=(((partition_table[(flags.drive_number-128)].extended_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048);
        } while( ( (100*maximum_partition_size_in_MB)/partition_table[(flags.drive_number-128)].extended_partition_size_in_MB ) >1);
    }
}
void Calculate_Partition_Ending_Cylinder(long start_cylinder,unsigned long size) {
    unsigned long cylinder_size=(partition_table[(flags.drive_number-0x80)].total_heads+1)*(partition_table[(flags.drive_number-0x80)].total_sectors);
    computed_partition_size=0;
    computed_ending_cylinder=start_cylinder;
    do {
        computed_ending_cylinder++;
        computed_partition_size=computed_partition_size+cylinder_size;
    } while(computed_partition_size<size);
    computed_ending_cylinder--;
    computed_partition_size=computed_partition_size-cylinder_size;
}
void Change_Current_Fixed_Disk_Drive() {
    int new_drive_number;
    int old_drive_number=flags.drive_number;
    Clear_Screen();
    Print_Centered(0,"Change Current Fixed Disk Drive",BOLD);
    Display_All_Drives();
    Position_Cursor(4,21);
    printf("Enter Fixed Disk Drive Number (1-%d).......................",(flags.maximum_drive_number-127));
    new_drive_number=Input(1,62,21,NUM,1,(flags.maximum_drive_number-127),ESCR,(flags.drive_number-127),0);
    if( (new_drive_number<=0) || (new_drive_number>(flags.maximum_drive_number-127)) ) {
        flags.drive_number=old_drive_number;
    } else {
        flags.drive_number=new_drive_number+127;
    }
}
void Check_For_INT13_Extensions() {
    int carry=99;
    asm{
        mov ah,0x41
        mov bx,0x55aa
        mov dl,0x80
        int 0x13
        jnc carry_flag_not_set    /* Jump if the carry flag is clear  */
    }                         /* If the carry flag is clear, then */
    carry=1;
    flags.extended_int_13=FALSE;
carry_flag_not_set:
    if(carry==99) flags.extended_int_13=TRUE;
}
void Clear_Active_Partition() {
    int index=0;
    do {
        partition_table[(flags.drive_number-128)].active_status[index]=0x00;
        index++;
    } while(index<4);
    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
    flags.partitions_have_changed=TRUE;
}
void Clear_Partition_Table() {
    Clear_Sector_Buffer();
    Write_Physical_Sector(flags.drive_number, 0, 0, 1);
}
void Clear_Partition_Table_Area_Of_Sector_Buffer() {
    long index=0x1be;
    do {
        sector_buffer[index]=0;
        index++;
    } while(index<=0x1fd);
}
void Clear_Screen() {
    asm{
        mov ah,0
        mov al,3
        int 0x10
    }
    Display_Information();
    Display_Label();
}
void Clear_Sector_Buffer() {
    int index=0;
    do {
        sector_buffer[index]=0;
        index ++;
    } while(index<512);
}
unsigned long Combine_Cylinder_and_Sector(unsigned long cylinder, unsigned long sector) {
    long value = 0;
    asm{
        mov ax,WORD PTR cylinder
        mov bx,WORD PTR sector
        mov dl,ah
        shl dl,1
        shl dl,1
        shl dl,1
        shl dl,1
        shl dl,1
        shl dl,1
        mov dh,al
        add dx,bx
        mov WORD PTR value,dx
    }
    return(value);
}
void Convert_Long_To_Integer(long number) {
    integer1=0;
    integer2=0;
    asm{
        mov ax,WORD PTR number
        mov BYTE PTR integer1, al
        mov BYTE PTR integer2, ah
    }
}
void Create_Alternate_MBR() {
    char home_path[255];
    long index=0;
    FILE *file_pointer;
    Read_Physical_Sector(flags.drive_number,0,0,1);
    do {
        sector_buffer[index]=0x00;
        index++;
    } while(index<0x1be);
    index=0;
    strcpy(home_path,path);
    strcat(home_path,"boot.mbr");
    file_pointer=fopen(home_path,"rb");
    if(!file_pointer) file_pointer=fopen(searchpath("boot.mbr"),"rb");
    if(!file_pointer) {
        printf("\nThe \"boot.mbr\" file has not been found...Operation Terminated.\n");
        exit(8);
    }
    do {
        sector_buffer[index]=fgetc(file_pointer);
        index++;
    } while(index<0x1be);
    fclose(file_pointer);
    sector_buffer[0x1fe]=0x55;
    sector_buffer[0x1ff]=0xaa;
    Write_Physical_Sector(flags.drive_number,0,0,1);
}
int Create_DOS_Partition_Interface(int type) {
    int numeric_type;
    int partition_created=FALSE;
    int partition_slot_just_used;
    long maximum_possible_percentage;
    unsigned long input=0;
    unsigned long maximum_partition_size_in_MB;
    Determine_Free_Space();
    maximum_partition_size_in_MB=(((partition_table[(flags.drive_number-128)].primary_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048);
    if( (type!=EXTENDED) && (flags.version==FOUR) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
    if( (type!=EXTENDED) && (flags.version==FIVE) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
    if( (type!=EXTENDED) && (flags.version==SIX) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
    if( (type!=EXTENDED) && (flags.version==W95) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
    if( (type!=EXTENDED) && ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==FALSE) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
    if(type==PRIMARY) {
        Clear_Screen();
        Print_Centered(4,"Create Primary DOS Partition",BOLD);
        Position_Cursor(4,6);
        printf("Current fixed disk drive: ");
        cprintf("%d",(flags.drive_number-127));
        Position_Cursor(4,8);
        printf("Do you wish to use the maximum available size for a Primary DOS Partition");
        Position_Cursor(4,9);
        printf("and make the partition active (Y/N).....................? ");
        flags.esc=FALSE;
        input=Input(1,62,9,YN,0,0,ESCR,1,0);
        if(flags.esc==TRUE) return(1);
        if(input==1) {
            input=maximum_partition_size_in_MB;
            if(input<=16) numeric_type=1;
            if( (input>16) && (input<=32) ) numeric_type=4;
            if(input>32) numeric_type=6;
            if( (input>512) && ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==TRUE) ) numeric_type=0x0b;
            partition_slot_just_used=Create_Primary_Partition(numeric_type,input);
            Set_Active_Partition(partition_slot_just_used);
            partition_created=TRUE;
        }
    }
    if(partition_created==FALSE) {
        Clear_Screen();
        if(type==PRIMARY) Print_Centered(4,"Create Primary DOS Partition",BOLD);
        else Print_Centered(4,"Create Extended DOS Partition",BOLD);
        Position_Cursor(4,6);
        printf("Current fixed disk drive: ");
        cprintf("%d",(flags.drive_number-127));
        Display_Primary_Partition_Information_SS();
        Position_Cursor(4,15);
        printf("Maximum space available for partition is ");
        cprintf("%4d",maximum_partition_size_in_MB);
        printf(" Mbytes ");
        maximum_possible_percentage=(100*maximum_partition_size_in_MB)/partition_table[(flags.drive_number-128)].total_hard_disk_size_in_MB;
        cprintf("(%3d%%)",maximum_possible_percentage);
        Position_Cursor(4,18);
        printf("Enter partition size in Mbytes or percent of disk space (%) to");
        Position_Cursor(4,19);
        if(type==PRIMARY) printf("create a Primary DOS Partition.................................: ");
        else printf("create an Extended DOS Partition...............................: ");
        flags.esc=FALSE;
        input=Input(4,69,19,NUMP,1,maximum_partition_size_in_MB,ESCR,maximum_partition_size_in_MB,maximum_possible_percentage);
        if(flags.esc==TRUE) return(1);
        if(type==PRIMARY) {
            if(input<=16) numeric_type=1;
            if( (input>16) && (input<=32) ) numeric_type=4;
            if(input>32) numeric_type=6;
            if( (input>512) && ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==TRUE) ) numeric_type=0x0b;
        } else numeric_type=5;
        Create_Primary_Partition(numeric_type,input);
    }
    Clear_Screen();
    if(type==PRIMARY) Print_Centered(4,"Create Primary DOS Partition",BOLD);
    else Print_Centered(4,"Create Extended DOS Partition",BOLD);
    Position_Cursor(4,6);
    printf("Current fixed disk drive: ");
    cprintf("%d",(flags.drive_number-127));
    Display_Primary_Partition_Information_SS();
    Position_Cursor(4,21);
    if(type==PRIMARY) cprintf("Primary DOS Partition created");
    else cprintf("Extended DOS Partition created");
    Input(0,0,0,ESC,0,0,ESCC,0,0);
    if(type==EXTENDED) Create_Logical_Drive_Interface();
    return(0);
}
int Create_Logical_Drive(int numeric_type, long size_in_MB) {
    int index;
    int offset;
    unsigned long maximum_size_in_logical_sectors=((partition_table[(flags.drive_number-128)].extended_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors));
    unsigned long maximum_size_in_MB=maximum_size_in_logical_sectors/2048;
    unsigned long size_in_logical_sectors;
    if(size_in_MB>maximum_size_in_MB) {
        size_in_MB=maximum_size_in_MB;
        size_in_logical_sectors=maximum_size_in_MB*2048;
        if(size_in_MB<=16) numeric_type=1;
        if( (size_in_MB>16) && (size_in_MB<=32) ) numeric_type=4;
        if(size_in_MB>32) numeric_type=6;
        if( (size_in_MB>512) && (flags.version==W95B) && (flags.fat32==TRUE) ) numeric_type=0x0b;
    } else {
        size_in_logical_sectors=size_in_MB*2048;
    }
    if( (partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location<=partition_table[(flags.drive_number-128)].number_of_logical_drives) && (partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location>0) ) {
        index=partition_table[(flags.drive_number-128)].number_of_logical_drives+1;
        do {
            partition_table[(flags.drive_number-128)].logical_drive_numeric_type[index]=partition_table[(flags.drive_number-128)].logical_drive_numeric_type[(index-1)];
            strcpy(partition_table[(flags.drive_number-128)].logical_drive_volume_label[index],partition_table[(flags.drive_number-128)].logical_drive_volume_label[(index-1)]);
            partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[index]=partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_starting_head[index]=partition_table[(flags.drive_number-128)].logical_drive_starting_head[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_starting_sector[index]=partition_table[(flags.drive_number-128)].logical_drive_starting_sector[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[index]=partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_ending_head[index]=partition_table[(flags.drive_number-128)].logical_drive_ending_head[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_ending_sector[index]=partition_table[(flags.drive_number-128)].logical_drive_ending_sector[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[index]=partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[index]=partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[(index-1)];
            partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[index]=partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_exists[index]=partition_table[(flags.drive_number-128)].next_extended_exists[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_numeric_type[index]=partition_table[(flags.drive_number-128)].next_extended_numeric_type[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[index]=partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_starting_head[index]=partition_table[(flags.drive_number-128)].next_extended_starting_head[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_starting_sector[index]=partition_table[(flags.drive_number-128)].next_extended_starting_sector[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[index]=partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_ending_head[index]=partition_table[(flags.drive_number-128)].next_extended_ending_head[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_ending_sector[index]=partition_table[(flags.drive_number-128)].next_extended_ending_sector[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_relative_sectors[index]=partition_table[(flags.drive_number-128)].next_extended_relative_sectors[(index-1)];
            partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[index]=partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[(index-1)];
            index--;
        } while(index>=partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location);
    }
    partition_table[(flags.drive_number-128)].logical_drive_numeric_type[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=numeric_type;
    strcpy(partition_table[(flags.drive_number-128)].logical_drive_volume_label[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location],"");
    partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].logical_drive_free_space_starting_cylinder;
    partition_table[(flags.drive_number-128)].logical_drive_starting_head[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=1;
    partition_table[(flags.drive_number-128)].logical_drive_starting_sector[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=1;
    Calculate_Partition_Ending_Cylinder(partition_table[(flags.drive_number-128)].logical_drive_free_space_starting_cylinder,size_in_logical_sectors);
    partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=computed_ending_cylinder;
    partition_table[(flags.drive_number-128)].logical_drive_ending_head[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].total_heads;
    partition_table[(flags.drive_number-128)].logical_drive_ending_sector[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].total_sectors;
    partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].total_sectors;
    partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=computed_partition_size;
    partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=computed_partition_size/2048;
    partition_table[(flags.drive_number-128)].number_of_logical_drives++;
    if(partition_table[(flags.drive_number-128)].logical_drive_numeric_type[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location+1)]>0) {
        partition_table[(flags.drive_number-128)].next_extended_exists[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=TRUE;
        partition_table[(flags.drive_number-128)].next_extended_numeric_type[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=5;
        partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location+1)];
        partition_table[(flags.drive_number-128)].next_extended_starting_head[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=0;
        partition_table[(flags.drive_number-128)].next_extended_starting_sector[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=1;
        partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location+1)];
        partition_table[(flags.drive_number-128)].next_extended_ending_head[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].total_heads;
        partition_table[(flags.drive_number-128)].next_extended_ending_sector[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].total_sectors;
        partition_table[(flags.drive_number-128)].next_extended_relative_sectors[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=(partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location+1)]-partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[partition_table[(flags.drive_number-128)].number_of_extended_partition])*(partition_table[(flags.drive_number-128)].total_heads+1)*partition_table[(flags.drive_number-128)].total_sectors;
        partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location]=partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location+1)]+partition_table[(flags.drive_number-128)].total_sectors;
    }
    if(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location>0) {
        partition_table[(flags.drive_number-128)].next_extended_exists[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=TRUE;
        partition_table[(flags.drive_number-128)].next_extended_numeric_type[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=5;
        partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=partition_table[(flags.drive_number-128)].logical_drive_free_space_starting_cylinder;
        partition_table[(flags.drive_number-128)].next_extended_starting_head[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=0;
        partition_table[(flags.drive_number-128)].next_extended_starting_sector[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=1;
        partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=computed_ending_cylinder;
        partition_table[(flags.drive_number-128)].next_extended_ending_head[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=partition_table[(flags.drive_number-128)].total_heads;
        partition_table[(flags.drive_number-128)].next_extended_ending_sector[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=partition_table[(flags.drive_number-128)].total_sectors;
        partition_table[(flags.drive_number-128)].next_extended_relative_sectors[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location)-1]=( (partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]) - (partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[partition_table[(flags.drive_number-128)].number_of_extended_partition]) )*(partition_table[(flags.drive_number-128)].total_heads+1)*partition_table[(flags.drive_number-128)].total_sectors;
        partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[(partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1)]=computed_partition_size+partition_table[(flags.drive_number-128)].total_sectors;
    }
    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
    flags.partitions_have_changed=TRUE;
    if(debug.create_partition==TRUE) {
        Clear_Screen();
        Print_Centered(1,"Create Logical Drive Debug",BOLD);
        Position_Cursor(4,3);
        printf("int numeric_type=%d",numeric_type);
        Position_Cursor(4,4);
        printf("long size_in_MB=%d",size_in_MB);
        Position_Cursor(4,5);
        printf("Number of partition that was created:  %d",partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location);
        Position_Cursor(4,7);
        printf("Brief logical drive table:");
        index=partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1;
        offset=9;
        Position_Cursor(4,8);
        printf(" #  NT     SC    SH    SS      EC   EH   ES      Rel. Sect.    Size in MB ");
        do {
            if( (index>=0) && (index<24) ) {
                Position_Cursor(4,offset);
                printf("%2d",index);
                Position_Cursor(7,offset);
                printf("%3d",partition_table[(flags.drive_number-128)].logical_drive_numeric_type[index]);
                Position_Cursor(13,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[index]);
                Position_Cursor(19,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].logical_drive_starting_head[index]);
                Position_Cursor(25,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].logical_drive_starting_sector[index]);
                Position_Cursor(33,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[index]);
                Position_Cursor(38,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].logical_drive_ending_head[index]);
                Position_Cursor(43,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].logical_drive_ending_sector[index]);
                Position_Cursor(58,offset);
                printf("%d",partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[index]);
                Position_Cursor(72,offset);
                printf("%5d",partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[index]);
            } else {
                Position_Cursor(4,offset);
                printf("N/A");
            }
            offset++;
            index++;
        } while(offset<=11);
        Position_Cursor(4,15);
        printf("Next extended location table:");
        Position_Cursor(4,16);
        printf(" #         SC    SH    SS      EC   EH   ES      Rel. Sect.    Size in MB ");
        index=partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location-1;
        offset=17;
        do {
            if( (index>=0) && (index<24) && (partition_table[(flags.drive_number-128)].next_extended_exists[index]==TRUE) ) {
                Position_Cursor(4,offset);
                printf("%2d",index);
                Position_Cursor(13,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[index]);
                Position_Cursor(19,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].next_extended_starting_head[index]);
                Position_Cursor(25,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].next_extended_starting_sector[index]);
                Position_Cursor(33,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[index]);
                Position_Cursor(38,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].next_extended_ending_head[index]);
                Position_Cursor(43,offset);
                printf("%4d",partition_table[(flags.drive_number-128)].next_extended_ending_sector[index]);
                Position_Cursor(72,offset);
                printf("%4d",((partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[index])/2048));
            } else {
                Position_Cursor(4,offset);
                printf("N/A");
            }
            offset++;
            index++;
        } while(offset<=19);
        Pause();
    }
    return(0);
}
int Create_Logical_Drive_Interface() {
    long input=0;
    int drive_created=FALSE;
    int maximum_possible_percentage;
    int numeric_type;
    unsigned long maximum_partition_size_in_MB;
    Determine_Free_Space();
    maximum_partition_size_in_MB=(((partition_table[(flags.drive_number-128)].extended_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048);
    do {
        if( (flags.version==FOUR) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
        if( (flags.version==FIVE) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
        if( (flags.version==SIX) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
        if( (flags.version==W95) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
        if( ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==FALSE) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
        Clear_Screen();
        if(drive_created==TRUE) {
            Position_Cursor(4,22);
            cprintf("Logical DOS Drive created, drive letters changed or added");
        }
        Print_Centered(1,"Create Logical DOS Drive in the Extended DOS Partition",BOLD);
        Display_Extended_Partition_Information_SS();
        if(1==Determine_Drive_Letters()) {
            Position_Cursor(4,22);
            cprintf("Maximum number of Logical DOS Drives installed.");
            Input(0,0,0,ESC,0,0,ESCC,0,0);
            return(1);
        }
        Position_Cursor(4,17);
        printf("Total Extended DOS Partition size is ");
        cprintf("%4d",partition_table[(flags.drive_number-128)].extended_partition_size_in_MB);
        printf(" Mbytes (1 Mbyte = 1048576 bytes)");
        Position_Cursor(4,18);
        printf("Maximum space available for partition is ");
        cprintf("%4d",maximum_partition_size_in_MB);
        printf(" Mbytes ");
        maximum_possible_percentage=(100*maximum_partition_size_in_MB)/partition_table[(flags.drive_number-128)].extended_partition_size_in_MB;
        cprintf("(%3d%%)",maximum_possible_percentage);
        Position_Cursor(4,20);
        printf("Enter logical drive size in Mbytes or percent of disk space (%)...");
        flags.esc=FALSE;
        input=Input(4,70,20,NUMP,1,maximum_partition_size_in_MB,ESCR,maximum_partition_size_in_MB,maximum_possible_percentage);
        if(flags.esc==TRUE) return(1);
        if(input<=16) numeric_type=1;
        if( (input>16) && (input<=32) ) numeric_type=4;
        if(input>32) numeric_type=6;
        if( (input>512) && ( (flags.version==W98) || (flags.version==W95B) ) && (flags.fat32==TRUE) ) numeric_type=0x0b;
        Create_Logical_Drive(numeric_type,input);
        drive_created=TRUE;
        Determine_Free_Space();
        maximum_partition_size_in_MB=(((partition_table[(flags.drive_number-128)].extended_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048);
    } while( ( (100*maximum_partition_size_in_MB)/partition_table[(flags.drive_number-128)].extended_partition_size_in_MB ) >1);
    Clear_Screen();
    Print_Centered(1,"Create Logical DOS Drive in the Extended DOS Partition",BOLD);
    Display_Extended_Partition_Information_SS();
    Position_Cursor(4,22);
    cprintf("All available space in the Extended DOS Partition");
    Position_Cursor(4,23);
    cprintf("is assigned to logical drives.");
    Input(0,0,0,ESC,0,0,ESCC,0,0);
    return(0);
}
void Create_MBR() {
    long number_of_bytes=SIZE_OF_MBR;
    long loop=0;
    if(flags.use_ambr==TRUE) {
        Create_Alternate_MBR();
    } else {
        Read_Physical_Sector(flags.drive_number,0,0,1);
        do {
            sector_buffer[loop]=boot_code[loop];
            loop++;
        } while(loop<=number_of_bytes);
        sector_buffer[0x1fe]=0x55;
        sector_buffer[0x1ff]=0xaa;
        Write_Physical_Sector(flags.drive_number,0,0,1);
    }
}
void Create_MBR_If_Not_Present() {
    Read_Physical_Sector(0x80,0,0,1);
    if( (sector_buffer[0x1fe]!=0x055) && (sector_buffer[0x1ff]!=0xaa) ) {
        Create_MBR();
    }
}
int Create_Primary_Partition(int numeric_type,long size_in_MB) {
    int index=0;
    int empty_partition_number=99;
    unsigned long maximum_size_in_logical_sectors=((partition_table[(flags.drive_number-128)].primary_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors));
    unsigned long maximum_size_in_MB=maximum_size_in_logical_sectors/2048;
    unsigned long size_in_logical_sectors;
    do {
        if( (empty_partition_number==99) && (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==0) ) {
            empty_partition_number=index;
        }
        index++;
    } while(index<4);
    if(empty_partition_number==99) return(99);
    if(size_in_MB>maximum_size_in_MB) {
        size_in_MB=maximum_size_in_MB;
        size_in_logical_sectors=maximum_size_in_MB*2048;
        if(numeric_type!=5) {
            if(size_in_MB<=16) numeric_type=1;
            if( (size_in_MB>16) && (size_in_MB<=32) ) numeric_type=4;
            if(size_in_MB>32) numeric_type=6;
            if( (size_in_MB>512) && (flags.version==W95B) && (flags.fat32==TRUE) ) numeric_type=0x0b;
        }
    } else {
        size_in_logical_sectors=size_in_MB*2048;
    }
    Calculate_Partition_Ending_Cylinder(partition_table[(flags.drive_number-128)].pp_largest_free_space_starting_cylinder,size_in_logical_sectors);
    partition_table[(flags.drive_number-128)].active_status[empty_partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_numeric_type[empty_partition_number]=numeric_type;
    partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[empty_partition_number]=partition_table[(flags.drive_number-128)].pp_largest_free_space_starting_cylinder;
    if(partition_table[(flags.drive_number-128)].pp_largest_free_space_starting_cylinder==0) partition_table[(flags.drive_number-128)].primary_partition_starting_head[empty_partition_number]=1;
    else partition_table[(flags.drive_number-128)].primary_partition_starting_head[empty_partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_starting_sector[empty_partition_number]=1;
    partition_table[(flags.drive_number-128)].primary_partition_ending_cylinder[empty_partition_number]=computed_ending_cylinder;
    partition_table[(flags.drive_number-128)].primary_partition_ending_head[empty_partition_number]=partition_table[(flags.drive_number-128)].total_heads;
    partition_table[(flags.drive_number-128)].primary_partition_ending_sector[empty_partition_number]=partition_table[(flags.drive_number-128)].total_sectors;
    if(partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[empty_partition_number]>0) {
        partition_table[(flags.drive_number-128)].primary_partition_relative_sectors[empty_partition_number]=(partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[empty_partition_number])*(partition_table[(flags.drive_number-128)].total_heads+1)*partition_table[(flags.drive_number-128)].total_sectors;
    } else partition_table[(flags.drive_number-128)].primary_partition_relative_sectors[empty_partition_number]=partition_table[(flags.drive_number-128)].total_sectors;
    partition_table[(flags.drive_number-128)].primary_partition_number_of_sectors[empty_partition_number]=computed_partition_size;
    partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[empty_partition_number]=size_in_MB;
    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
    flags.partitions_have_changed=TRUE;
    if(numeric_type==5) {
        partition_table[(flags.drive_number-128)].extended_partition_exists=TRUE;
        partition_table[(flags.drive_number-128)].number_of_extended_partition=empty_partition_number;
        partition_table[(flags.drive_number-128)].extended_partition_number_of_sectors=computed_partition_size;
        partition_table[(flags.drive_number-128)].extended_partition_size_in_MB=size_in_MB;
    }
    if(debug.create_partition==TRUE) {
        Clear_Screen();
        Print_Centered(1,"Create Primary Partition Debug",BOLD);
        Position_Cursor(4,3);
        printf("int numeric_type=%d",numeric_type);
        Position_Cursor(4,4);
        printf("long size_in_MB=%d",size_in_MB);
        Position_Cursor(4,5);
        printf("empty_partition_number=%d",empty_partition_number);
        Position_Cursor(4,8);
        printf("New Partition Information:");
        Position_Cursor(4,10);
        printf("Starting Cylinder:  %d",partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[empty_partition_number]);
        Position_Cursor(4,11);
        printf("Starting Head:      %d",partition_table[(flags.drive_number-128)].primary_partition_starting_head[empty_partition_number]);
        Position_Cursor(4,12);
        printf("Starting Sector:    %d",partition_table[(flags.drive_number-128)].primary_partition_starting_sector[empty_partition_number]);
        Position_Cursor(40,10);
        printf("Ending Cylinder:    %d",partition_table[(flags.drive_number-128)].primary_partition_ending_cylinder[empty_partition_number]);
        Position_Cursor(40,11);
        printf("Ending Head:        %d",partition_table[(flags.drive_number-128)].primary_partition_ending_head[empty_partition_number]);
        Position_Cursor(40,12);
        printf("Ending Sector:      %d",partition_table[(flags.drive_number-128)].primary_partition_ending_sector[empty_partition_number]);
        Position_Cursor(4,14);
        printf("Relative Sectors:   %d",partition_table[(flags.drive_number-128)].primary_partition_relative_sectors[empty_partition_number]);
        Position_Cursor(40,14);
        printf("Size of partition in MB:    %d",partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[empty_partition_number]);
        Pause();
    }
    return(empty_partition_number);
}
unsigned long Decimal_Number(unsigned long hex1, unsigned long hex2, unsigned long hex3, unsigned long hex4) {
    return((hex1) + (hex2*256) + (hex3*65536) + (hex4*16777216));
}
void Delete_Extended_DOS_Partition_Interface() {
    int input=0;
    Clear_Screen();
    Print_Centered(4,"Delete Extended DOS Partition",BOLD);
    Display_Primary_Partition_Information_SS();
    Position_Cursor(4,18);
    if(flags.monochrome!=TRUE) textcolor(WHITE | BLINK);
    cprintf("WARNING!");
    if(flags.monochrome!=TRUE) textcolor(15);
    printf(" Data in the deleted Extended DOS Partition will be lost.");
    Position_Cursor(4,19);
    printf("Do you wish to continue (Y/N).................? ");
    flags.esc=FALSE;
    input=Input(1,52,19,YN,0,0,ESCR,0,0);
    if( (flags.esc==FALSE) && (input==TRUE) ) {
        Delete_Primary_Partition(partition_table[(flags.drive_number-128)].number_of_extended_partition);
        Position_Cursor(4,21);
        cprintf("Extended DOS Partition deleted");
        Position_Cursor(4,24);
        printf("                                    ");
        Input(0,0,0,ESC,0,0,ESCC,0,0);
    }
}
void Delete_Logical_Drive(int logical_drive_number) {
    int index;
    partition_table[(flags.drive_number-128)].logical_drive_numeric_type[logical_drive_number]=0;
    strcpy(partition_table[(flags.drive_number-128)].logical_drive_volume_label[logical_drive_number],"           ");
    partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_starting_head[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_starting_sector[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_ending_head[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_ending_sector[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[logical_drive_number]=0;
    partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[logical_drive_number]=0;
    if(logical_drive_number>0) {
        partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[logical_drive_number];
        partition_table[(flags.drive_number-128)].next_extended_starting_head[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_starting_head[logical_drive_number];
        partition_table[(flags.drive_number-128)].next_extended_starting_sector[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_starting_sector[logical_drive_number];
        partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[logical_drive_number];
        partition_table[(flags.drive_number-128)].next_extended_ending_head[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_ending_head[logical_drive_number];
        partition_table[(flags.drive_number-128)].next_extended_ending_sector[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_ending_sector[logical_drive_number];
        partition_table[(flags.drive_number-128)].next_extended_relative_sectors[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_relative_sectors[logical_drive_number];
        partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[(logical_drive_number-1)]=partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[logical_drive_number];
        index=logical_drive_number;
        do {
            partition_table[(flags.drive_number-128)].logical_drive_numeric_type[index]=partition_table[(flags.drive_number-128)].logical_drive_numeric_type[(index+1)];
            strcpy(partition_table[(flags.drive_number-128)].logical_drive_volume_label[index],partition_table[(flags.drive_number-128)].logical_drive_volume_label[(index+1)]);
            partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[index]=partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_starting_head[index]=partition_table[(flags.drive_number-128)].logical_drive_starting_head[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_starting_sector[index]=partition_table[(flags.drive_number-128)].logical_drive_starting_sector[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[index]=partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_ending_head[index]=partition_table[(flags.drive_number-128)].logical_drive_ending_head[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_ending_sector[index]=partition_table[(flags.drive_number-128)].logical_drive_ending_sector[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[index]=partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[index]=partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[(index+1)];
            partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[index]=partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_numeric_type[index]=partition_table[(flags.drive_number-128)].next_extended_numeric_type[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[index]=partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_starting_head[index]=partition_table[(flags.drive_number-128)].next_extended_starting_head[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_starting_sector[index]=partition_table[(flags.drive_number-128)].next_extended_starting_sector[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[index]=partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_ending_head[index]=partition_table[(flags.drive_number-128)].next_extended_ending_head[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_ending_sector[index]=partition_table[(flags.drive_number-128)].next_extended_ending_sector[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_relative_sectors[index]=partition_table[(flags.drive_number-128)].next_extended_relative_sectors[(index+1)];
            partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[index]=partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[(index+1)];
            if(partition_table[(flags.drive_number-128)].next_extended_numeric_type[index]>0) {
                partition_table[(flags.drive_number-128)].next_extended_exists[(index-1)]=TRUE;
            } else {
                partition_table[(flags.drive_number-128)].next_extended_exists[(index-1)]=FALSE;
            }
            index++;
        } while(index<22);
    }
    partition_table[(flags.drive_number-128)].number_of_logical_drives--;
    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
    flags.partitions_have_changed=TRUE;
}
int Delete_Logical_Drive_Interface() {
    int drive_to_delete=0;
    int index=0;
    int input=0;
    int input_ok;
    Clear_Screen();
    Print_Centered(1,"Delete Logical DOS Drive(s) in the Extended DOS Partition",BOLD);
    Display_Extended_Partition_Information_SS();
    Position_Cursor(4,19);
    if(flags.monochrome!=TRUE) textcolor(WHITE | BLINK);
    cprintf("WARNING!");
    if(flags.monochrome!=TRUE) textcolor(15);
    printf(" Data in a deleted Logical DOS Drive will be lost.");
    Position_Cursor(4,20);
    printf ("What drive do you want to delete...............................? ");
    Determine_Drive_Letters();
    input_ok=FALSE;
    do {
        flags.esc=FALSE;
        input=Input(1,69,20,CHAR,68,90,ESCR,0,0);
        if(flags.esc==TRUE) return(1);
        index=4;
        do {
            if( (drive_lettering_buffer[(flags.drive_number-128)] [index]>0) && (drive_lettering_buffer[(flags.drive_number-128)] [index]==input) ) {
                input=index-4;
                input_ok=TRUE;
                index=30; /* break out of the loop */
            }
            index++;
        } while(index<=26);
    } while(input_ok==FALSE);
    drive_to_delete=input;
    Position_Cursor(4,22);
    printf("Are you sure (Y/N)..............................? ");
    flags.esc=FALSE;
    input=Input(1,54,22,YN,0,0,ESCR,0,0);
    if( (input==TRUE) && (flags.esc==FALSE) ) {
        Delete_Logical_Drive(drive_to_delete);
        Clear_Screen();
        Print_Centered(1,"Delete Logical DOS Drive(s) in the Extended DOS Partition",BOLD);
        Display_Extended_Partition_Information_SS();
        input=Input(0,0,0,ESC,0,0,ESCC,0,0);
    }
    return(0);
}
void Delete_N_DOS_Partition_Interface() {
    int input=0;
    Clear_Screen();
    Print_Centered(4,"Delete Non-DOS Partition",BOLD);
    Display_Primary_Partition_Information_SS();
    Position_Cursor(4,18);
    if(flags.monochrome!=TRUE) textcolor(WHITE | BLINK);
    cprintf("WARNING!");
    if(flags.monochrome!=TRUE) textcolor(15);
    printf(" Data in the deleted Non-DOS Partition will be lost.");
    Position_Cursor(4,19);
    printf("What Non-DOS Partition do you want to delete..? ");
    flags.esc=FALSE;
    input=Input(1,52,19,NUM,1,4,ESCR,-1,0); /* 4 needs changed to the max num of partitions */
    if(flags.esc==FALSE) {
        Delete_Primary_Partition(input-1);
        Position_Cursor(4,21);
        cprintf("Non-DOS Partition deleted");
        Position_Cursor(4,24);
        printf("                                    ");
        Input(0,0,0,ESC,0,0,ESCC,0,0);
    }
}
void Delete_Primary_Partition(int partition_number) {
    int index;
    if(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]==5) {
        partition_table[(flags.drive_number-128)].extended_partition_exists=FALSE;
        partition_table[(flags.drive_number-128)].number_of_extended_partition=0;
        partition_table[(flags.drive_number-128)].extended_partition_size_in_MB=0;
        partition_table[(flags.drive_number-128)].extended_partition_number_of_sectors=0;
        partition_table[(flags.drive_number-128)].extended_partition_largest_free_space=0;
        partition_table[(flags.drive_number-128)].logical_drive_largest_free_space_location=0;
        partition_table[(flags.drive_number-128)].logical_drive_free_space_starting_cylinder=0;
        partition_table[(flags.drive_number-128)].logical_drive_free_space_ending_cylinder=0;
        partition_table[(flags.drive_number-128)].number_of_logical_drives=0;
        index=0;
        do {
            partition_table[(flags.drive_number-128)].logical_drive_numeric_type[index]=0;
            strcpy(partition_table[(flags.drive_number-128)].logical_drive_volume_label[index],"            ");;
            partition_table[(flags.drive_number-128)].logical_drive_starting_cylinder[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_starting_head[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_starting_sector[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_ending_cylinder[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_ending_head[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_ending_sector[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_relative_sectors[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[index]=0;
            partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_exists[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_numeric_type[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_starting_cylinder[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_starting_head[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_starting_sector[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_ending_cylinder[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_ending_head[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_ending_sector[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_relative_sectors[index]=0;
            partition_table[(flags.drive_number-128)].next_extended_number_of_sectors[index]=0;
            index++;
        } while(index<23);
    }
    partition_table[(flags.drive_number-128)].active_status[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_starting_head[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_starting_sector[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_ending_cylinder[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_ending_head[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_ending_sector[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_relative_sectors[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_number_of_sectors[partition_number]=0;
    partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[partition_number]=0;
    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
    flags.partitions_have_changed=TRUE;
    strcpy(partition_table[(flags.drive_number-128)].primary_partition_volume_label[partition_number],"           ");
}
void Delete_Primary_DOS_Partition_Interface() {
    int input=0;
    int partition_to_delete;
    Clear_Screen();
    Print_Centered(4,"Delete Primary DOS Partition",BOLD);
    Display_Primary_Partition_Information_SS();
    Position_Cursor(4,19);
    if(flags.monochrome!=TRUE) textcolor(WHITE | BLINK);
    cprintf("WARNING!");
    if(flags.monochrome!=TRUE) textcolor(15);
    printf(" Data in the deleted Primary DOS Partition will be lost.");
    Position_Cursor(4,20);
    printf("What primary partition do you want to delete..? ");
    flags.esc=FALSE;
    input=Input(1,52,20,NUM,1,4,ESCR,-1,0); /* 4 needs changed to the max num of partitions */
    if(flags.esc==FALSE) {
        partition_to_delete=input-1;
        Position_Cursor(4,22);
        printf("Are you sure (Y/N)..............................? ");
        flags.esc=FALSE;
        input=Input(1,54,22,YN,0,0,ESCR,0,0);
        if( (input==TRUE) && (flags.esc==FALSE) ) {
            Delete_Primary_Partition(partition_to_delete);
            Clear_Screen();
            Print_Centered(4,"Delete Primary DOS Partition",BOLD);
            Display_Primary_Partition_Information_SS();
            Position_Cursor(4,21);
            cprintf("Primary DOS Partition deleted");
            Input(0,0,0,ESC,0,0,ESCC,0,0);
        }
    }
}
int Determine_Drive_Letters() {
    int current_letter='C';
    int drive_found=FALSE;
    int index=0;
    int sub_index=0;
    int first=FALSE;
    Load_Brief_Partition_Table();
    do {
        sub_index=0;
        do {
            drive_lettering_buffer[index] [sub_index]=0;
            sub_index++;
        } while(sub_index<27);
        index++;
    } while(index<8);
    index=0;
    do {
        sub_index=0;
        drive_found=FALSE;
        do {
            if( (brief_partition_table[index] [sub_index]==1) || (brief_partition_table[index] [sub_index]==4) || (brief_partition_table[index] [sub_index]==6) ) {
                drive_lettering_buffer[index] [sub_index]=current_letter;
                current_letter++;
                drive_found=TRUE;
            }
            if( (brief_partition_table[index] [sub_index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                drive_lettering_buffer[index] [sub_index]=current_letter;
                current_letter++;
                drive_found=TRUE;
            }
            sub_index++;
        } while( (sub_index<4) && (drive_found==FALSE) );
        index++;
    } while(index<8);
    index=0;
    do {
        sub_index=4;
        drive_found=FALSE;
        do {
            if( (brief_partition_table[index] [sub_index]==1) || (brief_partition_table[index] [sub_index]==4) || (brief_partition_table[index] [sub_index]==6) ) {
                drive_lettering_buffer[index] [sub_index]=current_letter;
                current_letter++;
            }
            if( (brief_partition_table[index] [sub_index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                drive_lettering_buffer[index] [sub_index]=current_letter;
                current_letter++;
            }
            sub_index++;
        } while(sub_index<27);
        index++;
    } while(index<8);
    index=0;
    do {
        sub_index=0;
        first=FALSE;
        do {
            if( (first==TRUE) && ( (brief_partition_table[index] [sub_index]==1) || (brief_partition_table[index] [sub_index]==4) || (brief_partition_table[index] [sub_index]==6) ) ) {
                drive_lettering_buffer[index] [sub_index]=current_letter;
                current_letter++;
            }
            if( (first==TRUE) && (brief_partition_table[index] [sub_index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                drive_lettering_buffer[index] [sub_index]=current_letter;
                current_letter++;
            }
            if( (first==FALSE) && ( (brief_partition_table[index] [sub_index]==1) || (brief_partition_table[index] [sub_index]==4) || (brief_partition_table[index] [sub_index]==6) ) ) {
                first=TRUE;
            }
            if( (first==FALSE) && (brief_partition_table[index] [sub_index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                first=TRUE;
            }
            sub_index++;
        } while(sub_index<4);
        index++;
    } while(index<8);
    if(current_letter>'Z') return(1);
    else return(0);
}
void Determine_Color_Video_Support() {
    asm{
        mov bx,es:0x10
        and bx,0x30
        cmp bx,0x30
        jne color
    }
    flags.monochrome=TRUE;
    textcolor(7);
    goto type_determined;
color:
    flags.monochrome=FALSE;
    textcolor(15);
type_determined:
}
void Determine_Free_Space() {
    int first_used_partition=UNUSED;
    int last_used_partition=UNUSED;
    int index;
    int sub_index;
    int swap;
    int drive=flags.drive_number-0x80;
    unsigned long free_space_after_last_used_partition=0;
    unsigned long free_space_before_first_used_partition=0;
    unsigned long free_space_between_partitions_0_and_1=0;
    unsigned long free_space_between_partitions_1_and_2=0;
    unsigned long free_space_between_partitions_2_and_3=0;
    unsigned long cylinder_size=(partition_table[drive].total_heads+1)*(partition_table[drive].total_sectors);
    index=0;
    do {
        partition_table[drive].primary_partition_physical_order[index]=index;
        index++;
    } while(index<4);
    index=0;
    do {
        sub_index=0;
        do {
            if(partition_table[drive].primary_partition_numeric_type[partition_table[drive].primary_partition_physical_order[sub_index]]==0) {
                swap=partition_table[drive].primary_partition_physical_order[sub_index];
                partition_table[drive].primary_partition_physical_order[sub_index]=partition_table[drive].primary_partition_physical_order[(sub_index+1)];
                partition_table[drive].primary_partition_physical_order[(sub_index+1)]=swap;
            }
            sub_index++;
        } while(sub_index<3);
        index++;
    } while(index<4);
    if(debug.determine_free_space==TRUE) {
        Clear_Screen();
        Print_Centered(0,"Determine Free Space Debug",BOLD);
        printf("\n\nCylinder Size (total heads * total sectors)=%d\n",cylinder_size);
        printf("\nContents after initial sorting of unused partitions to end:\n\n");
        index=0;
        do {
            printf("Partition %1d:  %1d    ",index,partition_table[drive].primary_partition_physical_order[index]);
            printf("SC:  %4d    ",partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[index]]);
            printf("EC:  %4d    ",partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[index]]);
            printf("Size in MB:  %4d\n",partition_table[drive].primary_partition_size_in_MB[partition_table[drive].primary_partition_physical_order[index]]);
            index++;
        } while(index<4);
        Position_Cursor(4,20);
        Pause();
    }
    index=0;
    do {
        sub_index=0;
        do {
            if( (partition_table[drive].primary_partition_numeric_type[partition_table[drive].primary_partition_physical_order[sub_index]]!=0) && (partition_table[drive].primary_partition_numeric_type[partition_table[drive].primary_partition_physical_order[(sub_index+1)]]!=0) && (partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[sub_index]]>partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[(sub_index+1)]]) ) {
                swap=partition_table[drive].primary_partition_physical_order[sub_index];
                partition_table[drive].primary_partition_physical_order[sub_index]=partition_table[drive].primary_partition_physical_order[(sub_index+1)];
                partition_table[drive].primary_partition_physical_order[(sub_index+1)]=swap;
            }
            sub_index++;
        } while(sub_index<3);
        index++;
    } while(index<4);
    if(debug.determine_free_space==TRUE) {
        Clear_Screen();
        Print_Centered(0,"Determine Free Space Debug",BOLD);
        printf("\n\nCylinder Size (total heads * total sectors)=%d\n",cylinder_size);
        printf("\nContents after sorting partitions by starting cylinder:\n\n");
        index=0;
        do {
            printf("Partition %d:  %1d    ",index,partition_table[drive].primary_partition_physical_order[index]);
            printf("SC:  %4d    ",partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[index]]);
            printf("EC:  %4d    ",partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[index]]);
            printf("Size in MB:  %4d\n",partition_table[drive].primary_partition_size_in_MB[partition_table[drive].primary_partition_physical_order[index]]);
            index++;
        } while(index<4);
    }
    index=0;
    do {
        if( (first_used_partition==UNUSED) && (partition_table[drive].primary_partition_numeric_type[partition_table[drive].primary_partition_physical_order[index]]>0) ) {
            first_used_partition=partition_table[drive].primary_partition_physical_order[index];
        }
        if(partition_table[drive].primary_partition_numeric_type[partition_table[drive].primary_partition_physical_order[index]]>0) {
            last_used_partition=partition_table[drive].primary_partition_physical_order[index];
        }
        index++;
    } while(index<4);
    if(first_used_partition!=UNUSED) {
        if(partition_table[drive].primary_partition_starting_cylinder[first_used_partition]>0) {
            free_space_before_first_used_partition=(partition_table[drive].primary_partition_starting_cylinder[first_used_partition]);
        } else free_space_before_first_used_partition=0;
    }
    if(first_used_partition!=UNUSED) {
        if(partition_table[drive].primary_partition_ending_cylinder[last_used_partition]<=partition_table[drive].total_cylinders) {
            free_space_after_last_used_partition=(partition_table[drive].total_cylinders-partition_table[drive].primary_partition_ending_cylinder[last_used_partition]);
        }
    }
    if( (first_used_partition!=UNUSED) && (last_used_partition>=1) ) {
        if( (partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[0]]+1)<(partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[1]]) ) {
            free_space_between_partitions_0_and_1=(partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[1]]-partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[0]]);
        }
    }
    if( (first_used_partition!=UNUSED) && (last_used_partition>=2) ) {
        if( (partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[1]]+1)<(partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[2]]) ) {
            free_space_between_partitions_1_and_2=(partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[2]]-partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[1]]);
        }
    }
    if( (first_used_partition!=UNUSED) && (last_used_partition==3) ) {
        if( (partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[2]]+1)<(partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[3]]) ) {
            free_space_between_partitions_2_and_3=(partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[3]]-partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[2]]);
        }
    }
    if(first_used_partition!=UNUSED) {
        partition_table[drive].pp_largest_free_space_starting_head=0;
        partition_table[drive].pp_largest_free_space_starting_sector=1;
        partition_table[drive].primary_partition_largest_free_space=free_space_before_first_used_partition;
        partition_table[drive].pp_largest_free_space_starting_cylinder=0;
        partition_table[drive].pp_largest_free_space_ending_cylinder=partition_table[drive].primary_partition_starting_cylinder[first_used_partition]-1;
        if(free_space_after_last_used_partition>partition_table[drive].primary_partition_largest_free_space) {
            partition_table[drive].primary_partition_largest_free_space=free_space_after_last_used_partition;
            partition_table[drive].pp_largest_free_space_starting_cylinder=partition_table[drive].primary_partition_ending_cylinder[last_used_partition]+1;
            partition_table[drive].pp_largest_free_space_ending_cylinder=partition_table[drive].total_cylinders;
        }
        if(free_space_between_partitions_0_and_1>partition_table[drive].primary_partition_largest_free_space) {
            partition_table[drive].primary_partition_largest_free_space=free_space_between_partitions_0_and_1;
            partition_table[drive].pp_largest_free_space_starting_cylinder=partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[0]]+1;
            partition_table[drive].pp_largest_free_space_ending_cylinder=partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[1]]-1;
        }
        if(free_space_between_partitions_1_and_2>partition_table[drive].primary_partition_largest_free_space) {
            partition_table[drive].primary_partition_largest_free_space=free_space_between_partitions_1_and_2;
            partition_table[drive].pp_largest_free_space_starting_cylinder=partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[1]]+1;
            partition_table[drive].pp_largest_free_space_ending_cylinder=partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[2]]-1;
        }
        if(free_space_between_partitions_2_and_3>partition_table[drive].primary_partition_largest_free_space) {
            partition_table[drive].primary_partition_largest_free_space=free_space_between_partitions_2_and_3;
            partition_table[drive].pp_largest_free_space_starting_cylinder=partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].primary_partition_physical_order[2]]+1;
            partition_table[drive].pp_largest_free_space_ending_cylinder=partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].primary_partition_physical_order[3]]-1;
        }
    } else {
        partition_table[drive].primary_partition_largest_free_space=partition_table[drive].total_cylinders;
        partition_table[drive].pp_largest_free_space_starting_cylinder=0;
        partition_table[drive].pp_largest_free_space_ending_cylinder=partition_table[drive].total_cylinders-1;
    }
    if(partition_table[drive].primary_partition_largest_free_space<=0) partition_table[drive].primary_partition_largest_free_space=0;
    if(debug.determine_free_space==TRUE) {
        printf("\n\nFree space (in cylinders) in primary partition table:\n");
        printf("\nFree space before first used partition:  %d",free_space_before_first_used_partition);
        printf("\nFree space after last used partition:  %d",free_space_after_last_used_partition);
        printf("\nFree space between partitions 0 and 1:  %d",free_space_between_partitions_0_and_1);
        printf("\nFree space between partitions 1 and 2:  %d",free_space_between_partitions_1_and_2);
        printf("\nFree space between partitions 2 and 3:  %d",free_space_between_partitions_2_and_3);
        printf("\n\nLargest free space in primary partition table:  %d",partition_table[drive].primary_partition_largest_free_space);
        printf("\nStarting cylinder of largest free space:  %d",partition_table[drive].pp_largest_free_space_starting_cylinder);
        printf("\nEnding cylinder of largest free space:  %d",partition_table[drive].pp_largest_free_space_ending_cylinder);
        Pause();
    }
    if(partition_table[drive].extended_partition_exists==TRUE) {
        partition_table[drive].extended_partition_largest_free_space=0;
        partition_table[drive].logical_drive_free_space_starting_cylinder=0;
        partition_table[drive].logical_drive_free_space_ending_cylinder=0;
        partition_table[drive].logical_drive_largest_free_space_location=0;
        if(debug.determine_free_space==TRUE) {
            Clear_Screen();
            Print_Centered(0,"Determine Free Space Debug",BOLD);
            printf("\n\n");
        }
        if(partition_table[drive].number_of_logical_drives>0) {
            last_used_partition=UNUSED;
            index=0;
            if(partition_table[drive].logical_drive_numeric_type[0]==0) index=1;
            do {
                if(partition_table[drive].logical_drive_numeric_type[index]>0) {
                    last_used_partition=index;
                }
                if( (partition_table[drive].logical_drive_starting_cylinder[(index+1)]-partition_table[drive].logical_drive_ending_cylinder[index]) >1 ) {
                    partition_table[drive].extended_partition_largest_free_space=(partition_table[drive].logical_drive_starting_cylinder[(index+1)]-1)-(partition_table[drive].logical_drive_ending_cylinder[index]+1);
                    partition_table[drive].logical_drive_free_space_starting_cylinder=partition_table[drive].logical_drive_ending_cylinder[index]+1;
                    partition_table[drive].logical_drive_free_space_ending_cylinder=partition_table[drive].logical_drive_starting_cylinder[(index+1)]-1;
                    partition_table[drive].logical_drive_largest_free_space_location=index+1;
                }
                if(debug.determine_free_space==TRUE) {
                    if(index==12) {
                        printf("\n");
                        Pause();
                    }
                    printf("\nLogical Drive #: %2d    ",index);
                    printf("SC: %4d    ",partition_table[drive].logical_drive_starting_cylinder[index]);
                    printf("EC: %4d    ",partition_table[drive].logical_drive_ending_cylinder[index]);
                }
                index++;
            } while(index<22);
            if(debug.determine_free_space==TRUE) {
                printf("\nLogical Drive #: %2d    ",index);
                printf("SC: %4d    ",partition_table[drive].logical_drive_starting_cylinder[22]);
                printf("EC: %4d    \n",partition_table[drive].logical_drive_ending_cylinder[22]);
                Pause();
                Clear_Screen();
                Print_Centered(0,"Determine Free Space Debug",BOLD);
                printf("\n\nNote:  All values are in cylinders.\n\n");
                printf("Results of free space calculations after computing spaces between\n  logical drives:\n\n");
                printf("Location of largest free space:  %d\n",partition_table[drive].logical_drive_largest_free_space_location);
                printf("Size of largest free space:  %4d\n",partition_table[drive].extended_partition_largest_free_space);
                printf("Starting cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_starting_cylinder);
                printf("Ending cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_ending_cylinder);
                Pause();
            }
            if(partition_table[drive].logical_drive_numeric_type[0]!=0) {
                if( (partition_table[drive].logical_drive_starting_cylinder[0]-partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition])>partition_table[drive].extended_partition_largest_free_space) {
                    partition_table[drive].extended_partition_largest_free_space=(partition_table[drive].logical_drive_starting_cylinder[0]-partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition]);
                    partition_table[drive].logical_drive_free_space_starting_cylinder=partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition];
                    partition_table[drive].logical_drive_free_space_ending_cylinder=partition_table[drive].logical_drive_starting_cylinder[0]-1;
                    partition_table[drive].logical_drive_largest_free_space_location=0;
                }
            } else {
                if( (partition_table[drive].logical_drive_starting_cylinder[1]-partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition])>partition_table[drive].extended_partition_largest_free_space) {
                    partition_table[drive].extended_partition_largest_free_space=(partition_table[drive].logical_drive_starting_cylinder[1]-partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition]);
                    partition_table[drive].logical_drive_free_space_starting_cylinder=partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition];
                    partition_table[drive].logical_drive_free_space_ending_cylinder=partition_table[drive].logical_drive_starting_cylinder[1]-1;
                    partition_table[drive].logical_drive_largest_free_space_location=0;
                }
            }
            if(debug.determine_free_space==TRUE) {
                Clear_Screen();
                Print_Centered(0,"Determine Free Space Debug",BOLD);
                printf("\n\nResults of free space calculations after computing space before\n  the first logical drive:\n\n");
                printf("Location of largest free space:  %d\n",partition_table[drive].logical_drive_largest_free_space_location);
                printf("Size of largest free space:  %4d\n",partition_table[drive].extended_partition_largest_free_space);
                printf("Starting cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_starting_cylinder);
                printf("Ending cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_ending_cylinder);
            }
            if( (last_used_partition<23) && (partition_table[drive].logical_drive_ending_cylinder[last_used_partition]<partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].number_of_extended_partition]) ) {
                if( (partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].number_of_extended_partition]-partition_table[drive].logical_drive_ending_cylinder[last_used_partition])>(partition_table[drive].extended_partition_largest_free_space) ) {
                    partition_table[drive].extended_partition_largest_free_space=(partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].number_of_extended_partition]-partition_table[drive].logical_drive_ending_cylinder[last_used_partition]);
                    partition_table[drive].logical_drive_free_space_starting_cylinder=partition_table[drive].logical_drive_ending_cylinder[last_used_partition]+1;
                    partition_table[drive].logical_drive_free_space_ending_cylinder=partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].number_of_extended_partition]-1;
                    partition_table[drive].logical_drive_largest_free_space_location=last_used_partition+1;
                }
            }
            if(debug.determine_free_space==TRUE) {
                printf("\n\nResults of free space calculations after computing space after\n  the last logical drive:\n\n");
                printf("Location of largest free space:  %d\n",partition_table[drive].logical_drive_largest_free_space_location);
                printf("Size of largest free space:  %4d\n",partition_table[drive].extended_partition_largest_free_space);
                printf("Starting cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_starting_cylinder);
                printf("Ending cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_ending_cylinder);
                Pause();
            }
        } else {
            partition_table[drive].extended_partition_largest_free_space=partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].number_of_extended_partition]-partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition]-1;
            partition_table[drive].logical_drive_free_space_starting_cylinder=partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition];
            partition_table[drive].logical_drive_free_space_ending_cylinder=partition_table[drive].primary_partition_ending_cylinder[partition_table[drive].number_of_extended_partition];
            if(debug.determine_free_space==TRUE) {
                printf("\n\nThere are not any Logical DOS Drives in the Extended DOS Partition\n\n");
                printf("Location of largest free space:     0\n");
                printf("Size of largest free space:  %d\n",partition_table[drive].extended_partition_largest_free_space);
                printf("Starting cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_starting_cylinder);
                printf("Ending cylinder of largest free space:  %d\n",partition_table[drive].logical_drive_free_space_ending_cylinder);
                Pause();
            }
        }
    }
}
void Display_All_Drives() {
    int current_column_offset_of_general_drive_information;
    int current_column_offset=4;
    int current_line=3;
    int current_line_of_general_drive_information;
    int drive=1;
    int drive_letter_index=0;
    int index;
    long space_used_on_drive_in_MB;
    unsigned long usage;
    Determine_Drive_Letters();
    Position_Cursor(2,2);
    printf("Disk   Drv   Mbytes   Free   Usage");
    do {
        if(current_line>18) {
            current_line=3;
            current_column_offset=45;
            Position_Cursor(43,2);
            printf("Disk   Drv   Mbytes   Free   Usage");
        }
        current_column_offset_of_general_drive_information=current_column_offset;
        current_line_of_general_drive_information=current_line;
        space_used_on_drive_in_MB=0;
        Position_Cursor(current_column_offset_of_general_drive_information,current_line);
        cprintf("%d",drive);
        Position_Cursor((current_column_offset_of_general_drive_information+12),current_line);
        printf("%4d",partition_table[(drive-1)].total_hard_disk_size_in_MB);
        index=0;
        do {
            if( (partition_table[(drive-1)].primary_partition_numeric_type[index]!=5) && (partition_table[(drive-1)].primary_partition_numeric_type[index]!=0) ) space_used_on_drive_in_MB=space_used_on_drive_in_MB+partition_table[(drive-1)].primary_partition_size_in_MB[index];
            index++;
        } while(index<=3);
        index=0;
        do {
            if(partition_table[(drive-1)].logical_drive_numeric_type[index]>0) space_used_on_drive_in_MB=space_used_on_drive_in_MB+partition_table[(drive-1)].logical_drive_size_in_MB[index];
            index++;
        } while(index<=22);
        drive_letter_index=0;
        do {
            if(drive_lettering_buffer[(drive-1)] [drive_letter_index]>0) {
                current_line++;
                if(current_line>18) {
                    current_line=3;
                    current_column_offset=45;
                    Position_Cursor(43,2);
                    printf("Disk   Drv   Mbytes   Free   Usage");
                }
                Position_Cursor((current_column_offset+6),current_line);
                printf("%c",drive_lettering_buffer[(drive-1)] [drive_letter_index]);
                Position_Cursor((current_column_offset+7),current_line);
                printf(":");
                Position_Cursor((current_column_offset+12),current_line);
                if(drive_letter_index<4) {
                    printf("%4d",partition_table[(drive-1)].primary_partition_size_in_MB[drive_letter_index]);
                } else {
                    printf("%4d",partition_table[(drive-1)].logical_drive_size_in_MB[(drive_letter_index-4)]);
                }
            }
            drive_letter_index++;
        } while(drive_letter_index<27);
        Position_Cursor((current_column_offset_of_general_drive_information+20),current_line_of_general_drive_information);
        printf("%4d",(partition_table[(drive-1)].total_hard_disk_size_in_MB-space_used_on_drive_in_MB));
        if(space_used_on_drive_in_MB==0) usage=0;
        else usage=1+((100*space_used_on_drive_in_MB)/(partition_table[(drive-1)].total_hard_disk_size_in_MB));
        if(usage>100) usage=100;
        Position_Cursor((current_column_offset_of_general_drive_information+28),current_line_of_general_drive_information);
        printf("%3d%%",usage);
        current_line++;
        drive++;
    } while(drive<=(flags.maximum_drive_number-127));
    Position_Cursor(4,20);
    printf("(1 Mbyte = 1048576 bytes)");
}
void Display_CL_Partition_Table() {
    int index=0;
    unsigned long usage=0;
    Determine_Drive_Letters();
    printf("\n\nCurrent fixed disk drive: %1d",(flags.drive_number-127));
    if(flags.extended_options_flag==TRUE) {
        printf("                  (TC: %4d",partition_table[(flags.drive_number-128)].total_cylinders);
        printf(" TH: %3d",partition_table[(flags.drive_number-128)].total_heads);
        printf(" TS: %3d)",partition_table[(flags.drive_number-128)].total_sectors);
    }
    printf("\n\nPartition   Status   Mbytes   Description     Usage  ");
    if(flags.extended_options_flag==TRUE) printf("Start Cyl  End Cyl");
    printf("\n");
    index=0;
    do {
        if(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]>0) {
            if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==1) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==4) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==6) || ( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) ) {
                printf(" %1c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
            } else printf("   ");
            printf(" %1d",(index+1));
            if(flags.extended_options_flag==TRUE) {
                printf(" %3d",(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]));
            } else printf("    ");
            if(partition_table[(flags.drive_number-128)].active_status[index]>0) {
                printf("      A");
            } else printf("       ");
            printf("    %6d",partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[index]);
            printf("   %-15d",partition_lookup_table_buffer_long[partition_table[(flags.drive_number-128)].primary_partition_numeric_type[(index)]]);
            usage=((partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[index]*100)/partition_table[(flags.drive_number-128)].total_hard_disk_size_in_MB);
            if(usage>100) usage=100;
            printf("                     %3d%%",usage);
            if(flags.extended_options_flag==TRUE) {
                printf("    %4d",partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[index]);
                printf("      %4d",partition_table[(flags.drive_number-128)].primary_partition_ending_cylinder[index]);
            }
            printf("\n");
        }
        index++;
    } while(index<4);
    if( (brief_partition_table[(flags.drive_number-128)] [4]>0) || (brief_partition_table[(flags.drive_number-128)] [5]>0) ) {
        printf("\nContents of Extended DOS Partition:\n");
        printf("Drv Volume Label  Mbytes  System  Usage\n");
        index=4;
        do {
            if( (brief_partition_table[(flags.drive_number-128)] [index]==1) || (brief_partition_table[flags.drive_number-128] [index]==4) || (brief_partition_table[flags.drive_number-128] [index]==6) ) {
                printf(" %1c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
                printf(" %11s",partition_table[(flags.drive_number-128)].logical_drive_volume_label[index-4]);
                printf("    %4d",partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[(index-4)]);
                printf("  %-8s",partition_lookup_table_buffer_short[partition_table[(flags.drive_number-128)].logical_drive_numeric_type[(index-4)]]);
                usage=((partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[index-4]*100)/partition_table[(flags.drive_number-128)].extended_partition_number_of_sectors);
                if(usage>100) usage=100;
                printf("  %3d%%",usage);
                printf("\n");
            }
            index++;
        } while(index<27);
    }
}
void Display_Help_Screen() {
    printf("FDISK switches:\n");
    printf("For partition table modification:\n");
    printf("  %s /A [drive#]\n",filename);
    printf("  %s /ACTIVATE <drive#> <partition#>\n",filename);
    printf("  %s /C [<drive#>]\n",filename);
    printf("  %s /D drive# {<partition#> | /P | /E | /L <drive_letter>} ]\n",filename);
    printf("  %s /DEACTIVATE <drive#>\n",filename);
    printf("  %s /M drive# <primary_partition#> <newtype#>\n",filename);
    printf("  %s /N <drive#> <partition_type> <size> [/P] [/S <type#>] ]\n",filename);
    printf("For MBR modification:\n");
    printf("  %s /AMBR [<drive#>]\n",filename);
    printf("  %s /MBR [<drive#>]\n",filename);
    printf("  %s /RMBR [<drive#>]\n",filename);
    printf("  %s /SMBR [<drive#>]\n",filename);
    printf("For obtaining information about the hard disk(s):\n");
    printf("  %s /DUMP\n",filename);
    printf("  %s /I [<drive#> [/TECH] ]\n",filename);
    printf("  %s /L\n",filename);
    printf("For rebooting the computer:\n");
    printf("  %s /REBOOT\n",filename);
    printf("\n");
}
void Display_Information() {
    if(flags.extended_options_flag==TRUE) {
        Position_Cursor(11,0);
        if(flags.fat32==TRUE) cprintf("FAT32");
        Position_Cursor(0,0);
        if(flags.version==THREETHREE) cprintf("3.3");
        if(flags.version==FOUR) cprintf("4");
        if(flags.version==FIVE) cprintf("5");
        if(flags.version==SIX) cprintf("6");
        if(flags.version==W95) cprintf("W95");
        if(flags.version==W95B) cprintf("W95B");
        if(flags.version==W98) cprintf("W98");
        Position_Cursor(5,0);
        if(flags.extended_int_13==TRUE) cprintf("INT13");
        Position_Cursor(79,0);
        if(flags.extended_options_flag==TRUE) cprintf("X");
        Position_Cursor(72,0);
        if(flags.use_ambr==TRUE) cprintf("AMBR");
        Position_Cursor(77,0);
        if(flags.partitions_have_changed==TRUE) cprintf("C");
    }
    if(debug.emulate_disk>0) {
        Position_Cursor(66,0);
        cprintf("E%1d",debug.emulate_disk);
    }
    if(debug.write==FALSE) {
        Position_Cursor(69,0);
        cprintf("RO");
    }
}
void Display_Label() {
    if(flags.label==TRUE) {
        int index=0;
        char label[20];
        strcpy(label,NAME);
        do {
            Position_Cursor(79,((index*2)+3));
            printf("%c",label[index]);
            index++;
        } while(index<10);
    }
}
void Display_Extended_Partition_Information_SS() {
    int column_index=0;
    int index;
    int print_index=4;
    unsigned long usage;
    Determine_Drive_Letters();
    if( (brief_partition_table[(flags.drive_number-128)] [4]>0) || (brief_partition_table[(flags.drive_number-128)] [5]>0) ) {
        Position_Cursor(0,3);
        printf("Drv Volume Label  Mbytes  System  Usage");
        index=4;
        print_index=4;
        do {
            if( (brief_partition_table[(flags.drive_number-128)] [index]==1) || (brief_partition_table[flags.drive_number-128] [index]==4) || (brief_partition_table[flags.drive_number-128] [index]==6) || ( (brief_partition_table[flags.drive_number-128] [index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) ) {
                if(print_index>15) {
                    column_index=41;
                    print_index=4;
                    Position_Cursor(41,3);
                    printf("Drv Volume Label  Mbytes  System  Usage");
                }
                Position_Cursor((column_index+0),print_index);
                cprintf("%c",drive_lettering_buffer[(flags.drive_number-128)] [index]);
                Position_Cursor((column_index+1),print_index);
                cprintf(":");
                Position_Cursor((column_index+4),print_index);
                printf("%11s",partition_table[(flags.drive_number-128)].logical_drive_volume_label[index-4]);
                Position_Cursor((column_index+19),print_index);
                printf("%4d",partition_table[(flags.drive_number-128)].logical_drive_size_in_MB[(index-4)]);
                Position_Cursor((column_index+25),print_index);
                printf("%s",partition_lookup_table_buffer_short[partition_table[(flags.drive_number-128)].logical_drive_numeric_type[(index-4)]]);
                usage=((partition_table[(flags.drive_number-128)].logical_drive_number_of_sectors[index-4]*100)/partition_table[(flags.drive_number-128)].extended_partition_number_of_sectors);
                if(usage>100) usage=100;
                Position_Cursor((column_index+35),print_index);
                printf("%3d%%",usage);
                print_index++;
            }
            index++;
        } while(index<27);
    } else {
        Position_Cursor(4,10);
        cprintf("No logical drives defined");
    }
    Position_Cursor(4,17);
    printf("Total Extended DOS Partition size is ");
    cprintf("%4d",(partition_table[flags.drive_number-128].extended_partition_size_in_MB) );
    printf(" Mbytes (1 Mbyte = 1048576 bytes)");
}
void Display_Or_Modify_Partition_Information() {
    int finished=FALSE;
    int input;
    int partition_number;
    unsigned long new_partition_size_in_sectors;
    unsigned long usage;
    do {
        Clear_Screen();
        Print_Centered(4,"Display/Modify Partition Information",BOLD);
        Display_Primary_Partition_Information_SS();
        Position_Cursor(4,18);
        printf("Which partition do you wish to modify...........................?");
        flags.esc=FALSE;
        partition_number=Input(1,69,18,NUM,1,4,ESCC,-1,0)-1;
        if(flags.esc==TRUE) finished=TRUE;
        else {
            Clear_Screen();
            Print_Centered(4,"Display/Modify Partition Information",BOLD);
            Determine_Drive_Letters();
            Position_Cursor(4,6);
            printf("Current fixed disk drive: ");
            cprintf("%d",(flags.drive_number-127));
            Position_Cursor(4,8);
            printf("Partition   Status   Mbytes    Description    Usage  Start Cyl  End Cyl");
            if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]==1) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]==4) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]==6) ) {
                Position_Cursor(5,9);
                printf("%c:",drive_lettering_buffer[(flags.drive_number-128)] [partition_number]);
            }
            if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                Position_Cursor(5,9);
                printf("%c:",drive_lettering_buffer[(flags.drive_number-128)] [partition_number]);
            }
            Position_Cursor(8,9);
            cprintf("%d",(partition_number+1));
            Position_Cursor(10,9);
            printf("%3d",(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]));
            if(partition_table[(flags.drive_number-128)].active_status[partition_number]>0) {
                Position_Cursor(19,9);
                printf("A");
            }
            Position_Cursor(24,9);
            printf("%6d",partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[partition_number]);
            Position_Cursor(33,9);
            printf("%15s",partition_lookup_table_buffer_long[partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]]);
            usage=((partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[partition_number]*100)/partition_table[(flags.drive_number-128)].total_hard_disk_size_in_MB);
            if(usage>100) usage=100;
            Position_Cursor(51,9);
            printf("%3d%%",usage);
            Position_Cursor(59,9);
            printf("%4d",partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[partition_number]);
            Position_Cursor(69,9);
            printf("%4d",partition_table[(flags.drive_number-128)].primary_partition_ending_cylinder[partition_number]);
            Position_Cursor(4,12);
            printf("Choose one of the following:");
            Position_Cursor(4,14);
            cprintf("1.");
            printf("  Change partition type");
            Position_Cursor(4,15);
            cprintf("2.");
            printf("  Remove active status");
            Position_Cursor(44,14);
            cprintf("3.");
            printf("  Hide/Unhide partition");
            Position_Cursor(44,15);
            cprintf("4.");
            printf("  Resize partition (destructive)");
            Position_Cursor(4,17);
            printf("Enter choice: ");
            flags.esc=FALSE;
            input=Input(1,19,17,NUM,1,4,ESCC,-1,0);
            if(flags.esc==TRUE) input=99;
            if(input==1) {
                Position_Cursor(4,19);
                printf("Enter new partition type (1-255)...................................");
                flags.esc=FALSE;
                input=Input(3,71,19,NUM,1,255,ESCC,-1,0);
                if(flags.esc==FALSE) {
                    Modify_Partition_Type(partition_number,input);
                    input=99;
                } else input=99;
            }
            if(input==2) Clear_Active_Partition();
            if(input==3) {
                if(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]>140) {
                    partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]=partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]-140;
                    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
                    flags.partitions_have_changed=TRUE;
                    input=99;
                }
                if( (input==3) && (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]<16) ) {
                    partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]=partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]+140;
                    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
                    flags.partitions_have_changed=TRUE;
                }
            }
            if(input==4) {
                Position_Cursor(4,19);
                printf("Enter new partition ending cylinder...............................");
                flags.esc=FALSE;
                input=Input(3,71,19,NUM,(partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[partition_number]+1),partition_table[(flags.drive_number-128)].total_cylinders,ESCC,-1,0);
                if(flags.esc==FALSE) {
                    new_partition_size_in_sectors=(input-partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[partition_number])*(partition_table[(flags.drive_number-128)].total_heads+1)*partition_table[(flags.drive_number-128)].total_sectors;
                    Calculate_Partition_Ending_Cylinder(partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[partition_number],new_partition_size_in_sectors);
                    partition_table[(flags.drive_number-128)].primary_partition_ending_cylinder[partition_number]=computed_ending_cylinder;
                    partition_table[(flags.drive_number-128)].primary_partition_number_of_sectors[partition_number]=computed_partition_size;
                    partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[partition_number]=computed_partition_size/2048;
                    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
                    flags.partitions_have_changed=TRUE;
                    input=99;
                } else input=99;
            }
        }
    } while(finished==FALSE);
    if(partition_table[(flags.drive_number-128)].number_of_logical_drives>0) {
        Position_Cursor(4,17);
        printf("The Extended DOS Partition contains Logical DOS Drives.");
        Position_Cursor(4,18);
        printf("                                                                       ");
        Position_Cursor(4,18);
        printf("Do you want to display the logical drive information (Y/N)......?");
        input=Input(1,69,18,YN,0,0,ESCR,1,0);
        if(input==TRUE) {
            asm{
                mov ah,0
                mov al,3
                int 0x10
            }
            Print_Centered(1,"Display Logical DOS Drive Information",BOLD);
            Display_Extended_Partition_Information_SS();
            Input(0,0,0,ESC,0,0,ESCC,0,0);
        }
    }
}
void Display_Partition_Information() {
    int input;
    Clear_Screen();
    Print_Centered(4,"Display Partition Information",BOLD);
    Display_Primary_Partition_Information_SS();
    if(partition_table[(flags.drive_number-128)].number_of_logical_drives>0) {
        Position_Cursor(4,17);
        printf("The Extended DOS Partition contains Logical DOS Drives.");
        Position_Cursor(4,18);
        printf("Do you want to display the logical drive information (Y/N)......?");
        input=Input(1,69,18,YN,0,0,ESCR,1,0);
        if(input==TRUE) {
            asm{
                mov ah,0
                mov al,3
                int 0x10
            }
            Print_Centered(1,"Display Logical DOS Drive Information",BOLD);
            Display_Extended_Partition_Information_SS();
            Input(0,0,0,ESC,0,0,ESCC,0,0);
        }
    } else {
        Input(0,0,0,ESC,0,0,ESCC,0,0);
    }
}
void Display_Primary_Partition_Information_SS() {
    int cursor_offset=0;
    int index=0;
    unsigned long usage=0;
    Determine_Drive_Letters();
    Position_Cursor(4,6);
    printf("Current fixed disk drive: ");
    cprintf("%d",(flags.drive_number-127));
    if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[0]>0) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[1]>0) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[2]>0) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[3]>0) ) {
        if(flags.extended_options_flag==FALSE) {
            Position_Cursor(4,8);
            printf("Partition  Status   Type    Volume Label  Mbytes   System   Usage");
            index=0;
            do {
                if(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]>0) {
                    if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==1) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==4) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==6) ) {
                        Position_Cursor(5,(cursor_offset+9));
                        printf("%c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
                    }
                    if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                        Position_Cursor(5,(cursor_offset+9));
                        printf("%c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
                    }
                    Position_Cursor(8,(cursor_offset+9));
                    cprintf("%d",(index+1));
                    if(partition_table[(flags.drive_number-128)].active_status[index]>0) {
                        Position_Cursor(18,(cursor_offset+9));
                        printf("A");
                    }
                    if(  (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==1) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==4) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==6)  ) {
                        Position_Cursor(23,(cursor_offset+9));
                        printf("PRI DOS");
                    }
                    if(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==5) {
                        Position_Cursor(23,(cursor_offset+9));
                        printf("EXT DOS");
                    }
                    if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]!=1) && (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]!=4) && (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]!=5) && (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]!=6)  ) {
                        Position_Cursor(23,(cursor_offset+9));
                        printf("Non-DOS");
                    }
                    if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                        Position_Cursor(23,(cursor_offset+9));
                        printf("PRI DOS");
                    }
                    Position_Cursor(33,(cursor_offset+9));
                    printf("%11s",partition_table[(flags.drive_number-128)].primary_partition_volume_label[index]);
                    Position_Cursor(45,(cursor_offset+9));
                    printf("%6d",partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[index]);
                    Position_Cursor(54,(cursor_offset+9));
                    printf("%s",partition_lookup_table_buffer_short[partition_table[(flags.drive_number-128)].primary_partition_numeric_type[(index)]]);
                    usage=((partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[index]*100)/partition_table[(flags.drive_number-128)].total_hard_disk_size_in_MB);
                    if(usage>100) usage=100;
                    Position_Cursor(65,(cursor_offset+9));
                    printf("%3d%%",usage);
                    cursor_offset++;
                }
                index++;
            } while(index<4);
        } else {
            Position_Cursor(4,8);
            printf("Partition   Status   Mbytes    Description    Usage  Start Cyl  End Cyl");
            index=0;
            do {
                if(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]>0) {
                    if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==1) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==4) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==6) ) {
                        Position_Cursor(5,(cursor_offset+9));
                        printf("%c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
                    }
                    if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) {
                        Position_Cursor(5,(cursor_offset+9));
                        printf("%c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
                    }
                    Position_Cursor(8,(cursor_offset+9));
                    cprintf("%d",(index+1));
                    Position_Cursor(10,(cursor_offset+9));
                    printf("%3d",(partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]));
                    if(partition_table[(flags.drive_number-128)].active_status[index]>0) {
                        Position_Cursor(19,(cursor_offset+9));
                        printf("A");
                    }
                    Position_Cursor(24,(cursor_offset+9));
                    printf("%6d",partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[index]);
                    Position_Cursor(33,(cursor_offset+9));
                    printf("%15s",partition_lookup_table_buffer_long[partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]]);
                    usage=((partition_table[(flags.drive_number-128)].primary_partition_size_in_MB[index]*100)/partition_table[(flags.drive_number-128)].total_hard_disk_size_in_MB);
                    if(usage>100) usage=100;
                    Position_Cursor(51,(cursor_offset+9));
                    printf("%3d%%",usage);
                    Position_Cursor(59,(cursor_offset+9));
                    printf("%4d",partition_table[(flags.drive_number-128)].primary_partition_starting_cylinder[index]);
                    Position_Cursor(69,(cursor_offset+9));
                    printf("%4d",partition_table[(flags.drive_number-128)].primary_partition_ending_cylinder[index]);
                    cursor_offset++;
                }
                index++;
            } while(index<4);
        }
    } else {
        Position_Cursor(4,21);
        cprintf("No partitions defined");
    }
    Position_Cursor(4,14);
    printf("Total disk space is ");
    cprintf("%4d",partition_table[(flags.drive_number-128)].total_hard_disk_size_in_MB);
    printf(" Mbytes (1 Mbyte = 1048576 bytes)");
}
void Dump_Partition_Information() {
    int index=0;
    flags.extended_options_flag=TRUE;
    do {
        flags.drive_number=index+128;
        Display_CL_Partition_Table();
        index++;
    } while(index<=7);
}
unsigned long Extract_Cylinder(unsigned long hex1, unsigned long hex2) {
    unsigned long cylinder_and_sector = ( (256*hex2) + hex1 );
    unsigned long extracted_cylinder = ( ( (cylinder_and_sector*4) & 768) + (cylinder_and_sector /256) );
    return(extracted_cylinder);
}
unsigned long Extract_Sector(unsigned long hex1, unsigned long hex2) {
    unsigned long cylinder_and_sector = ( (256*hex2) + hex1 );
    unsigned long extracted_sector = cylinder_and_sector % 64;
    return(extracted_sector);
}
void Get_Partition_Information() {
    int index;
    int sub_index;
    long sector_buffer_index;
    index=0;
    do {
        sub_index=0;
        do {
            if( (partition_table[index].primary_partition_numeric_type[sub_index]==1) || (partition_table[index].primary_partition_numeric_type[sub_index]==4) || (partition_table[index].primary_partition_numeric_type[sub_index]==6) ) {
                Read_Physical_Sector((index+128),partition_table[index].primary_partition_starting_cylinder[sub_index],partition_table[index].primary_partition_starting_head[sub_index],partition_table[index].primary_partition_starting_sector[sub_index]);
                if( (sector_buffer[54]>=48) && (sector_buffer[54]<=122) ) {
                    sector_buffer_index=43;
                    do {
                        partition_table[index].primary_partition_volume_label[sub_index] [(sector_buffer_index-43)]=sector_buffer[sector_buffer_index];
                        sector_buffer_index++;
                    } while(sector_buffer_index<54);
                }
            } else {
                strcpy(partition_table[index].primary_partition_volume_label[sub_index],"           ");
            }
            sub_index++;
        } while(sub_index<4);
        index++;
    } while(index<8);
    index=0;
    do {
        sub_index=0;
        do {
            if( (partition_table[index].logical_drive_numeric_type[sub_index]==1) || (partition_table[index].logical_drive_numeric_type[sub_index]==4) || (partition_table[index].logical_drive_numeric_type[sub_index]==6) ) {
                Read_Physical_Sector((index+128),partition_table[index].logical_drive_starting_cylinder[sub_index],partition_table[index].logical_drive_starting_head[sub_index],partition_table[index].logical_drive_starting_sector[sub_index]);
                if( (sector_buffer[54]>=48) && (sector_buffer[54]<=122) ) {
                    sector_buffer_index=43;
                    do {
                        partition_table[index].logical_drive_volume_label[sub_index] [(sector_buffer_index-43)]=sector_buffer[sector_buffer_index];
                        sector_buffer_index++;
                    } while(sector_buffer_index<54);
                }
            } else {
                strcpy(partition_table[index].logical_drive_volume_label[sub_index],"           ");
            }
            sub_index++;
        } while(sub_index<23);
        index++;
    } while(index<8);
}
void Initialization() {
    int value1;
    int value2;
    long index;
    unsigned long sector_buffer_segment;
    unsigned long sector_buffer_offset;
    flags.display_name_description_copyright=TRUE;
    flags.monochrome=FALSE;
    flags.partitions_have_changed=FALSE;
    flags.drive_number=128;
    if(flags.partition_type_lookup_table==INTERNAL) {
        index=1;
        do {
            if( (index!=5) && (index!=12) ) {
                strcpy(partition_lookup_table_buffer_short[index],"Unknown ");
                strcpy(partition_lookup_table_buffer_long[index],"Unknown        ");
            }
            index++;
        } while(index<=255);
        strcpy(partition_lookup_table_buffer_short[1],"FAT12   ");
        strcpy(partition_lookup_table_buffer_short[4],"FAT16   ");
        strcpy(partition_lookup_table_buffer_short[6],"FAT16   ");
        strcpy(partition_lookup_table_buffer_short[11],"FAT32   ");
        strcpy(partition_lookup_table_buffer_short[15],"FAT16   ");
        strcpy(partition_lookup_table_buffer_long[1],"FAT12          ");
        strcpy(partition_lookup_table_buffer_long[4],"FAT16          ");
        strcpy(partition_lookup_table_buffer_long[6],"FAT16          ");
        strcpy(partition_lookup_table_buffer_long[11],"FAT32          ");
        strcpy(partition_lookup_table_buffer_long[15],"FAT16     (LBA)");
    }
    Process_Fdiskini_File();
    if( (flags.version==W95) || (flags.version==W95B) || (flags.version==W98) )  Check_For_INT13_Extensions();
    else flags.extended_int_13=FALSE;
    if( (flags.version==W95B) || (flags.version==W98) ) flags.fat32=TRUE;
    Determine_Color_Video_Support();
    Read_Partition_Tables();
    if( (partition_table[1].total_cylinders+partition_table[2].total_cylinders+partition_table[3].total_cylinders+partition_table[4].total_cylinders+partition_table[5].total_cylinders+partition_table[6].total_cylinders+partition_table[7].total_cylinders)>0) {
        flags.more_than_one_drive=TRUE;
    } else flags.more_than_one_drive=FALSE;
    if(flags.extended_int_13==TRUE); /* Get rid of the ";" when ready.  */
    {
        disk_address_packet[0]=16;
        disk_address_packet[1]=0;
        disk_address_packet[2]=1;
        disk_address_packet[3]=0;
        *(long *)transfer_buffer_segment=FP_SEG(sector_buffer[0]);
        *(long *)transfer_buffer_offset=FP_OFF(sector_buffer[0]);
        value1=disk_address_packet[4];
        value2=disk_address_packet[5];
        disk_address_packet[4]=value2;
        disk_address_packet[5]=value1;
        value1=disk_address_packet[6];
        value2=disk_address_packet[7];
        disk_address_packet[6]=value2;
        disk_address_packet[7]=value1;
        disk_address_packet[8]=0;
        disk_address_packet[9]=0;
        disk_address_packet[10]=0;
        disk_address_packet[11]=0;
    }
}
unsigned long Input(int size_of_field,int x_position,int y_position,int type, int min_range, long max_range,int return_message,int default_value,long maximum_possible_percentage) {
    char input;
    char line_buffer[10];
    unsigned long multiplier;
    int index;
    int invalid_input=FALSE;
    int line_buffer_index=0;
    int proper_input_given=FALSE;
    int percent_entered=FALSE;
    int percent_just_entered=FALSE;
    unsigned long data_max_range=max_range;
    unsigned long data;
    index=0;
    do {
        line_buffer[index]=0;
        index++;
    } while(index<10);
    if(type!=ESC) {
        Position_Cursor(x_position,y_position);
        cprintf("[");
        index=0;
        do {
            cprintf(" ");
            index++;
        } while(index<size_of_field);
        cprintf("]");
    }
    if( (return_message==ESCR) || (return_message==ESCE) || (return_message==ESCC) ) {
        Position_Cursor(4,24);
        printf("Press ");
        cprintf("Esc");
        printf(" to ");
    }
    if(return_message==ESCR) printf("return to FDISK options");
    if(return_message==ESCE) printf("exit FDISK");
    if(return_message==ESCC) printf("continue");
    if( (default_value>=0) && (type==NUM) && (size_of_field==1) ) {
        Position_Cursor(x_position+1,y_position);
        printf("%d",default_value);
        line_buffer_index=0;
        line_buffer[0]=default_value+48;
    }
    if( (default_value>=0) && (type==NUM) && (size_of_field>1) ) {
    }
    if( (default_value>=0) && (type==YN) && (size_of_field==1) ) {
        Position_Cursor(x_position+1,y_position);
        if(default_value==1) {
            printf("Y");
            line_buffer_index=0;
            line_buffer[0]='Y';
            data=TRUE;
        }
        if(default_value==0) {
            printf("N");
            line_buffer_index=0;
            line_buffer[0]='N';
            data=FALSE;
        }
    }
    do {
        if(type!=ESC) Position_Cursor((size_of_field+x_position),y_position);
        asm{
            mov ah,7
            int 0x21
            mov BYTE PTR input,al
        }
        if(type!=YN) {
            Position_Cursor(4,22);
            printf("                                                            ");
        }
        Position_Cursor(4,23);
        printf("                                                    ");
        if(input==27) {
            flags.esc=TRUE;
            proper_input_given=TRUE;
            data=0;
            type=99;
        }
        if(input==13) {
            if( ( (type==CHAR) || (type==YN) ) && (line_buffer[0]!=0) && ( (data==TRUE) || (data==FALSE) || (data!=99) ) ) {
                proper_input_given=TRUE;
                type=99;
            }
            if( (type==NUM) && (line_buffer[0]!=0) ) {
                proper_input_given=TRUE;
                data=0;
                index=strlen(line_buffer)-1;
                multiplier=1;
                do {
                    data=data+((line_buffer[index]-48)*multiplier);
                    index--;
                    multiplier=multiplier*10;
                } while(index>=0);
                if(data>data_max_range) {
                    data=0;
                    proper_input_given=FALSE;
                    Position_Cursor(4,22);
                    cprintf("Requested partition size exceeds the maximum available space");
                    input='\xff';
                } else type=99;
            }
            if( (type==NUMP) && (line_buffer[0]!=0) ) {
                proper_input_given=TRUE;
                data=0;
                index=strlen(line_buffer)-1;
                if(percent_entered==TRUE) index--;
                multiplier=1;
                do {
                    data=data+((line_buffer[index]-48)*multiplier);
                    index--;
                    multiplier=multiplier*10;
                } while(index>=0);
                if(percent_entered==TRUE) data=(data*data_max_range)/maximum_possible_percentage;
                if(data>data_max_range) {
                    data=0;
                    proper_input_given=FALSE;
                    Position_Cursor(4,22);
                    cprintf("Requested partition size exceeds the maximum available space");
                    input='\xff';
                } else type=99;
            }
            if( (debug.input_routine==TRUE) && (type==99) ) {
                Clear_Screen();
                printf("Input entered by user:  %d",data);
                Pause();
            }
        }
        if(debug.input_routine==TRUE) {
            Position_Cursor(50,22);
            printf("                  ");
            Position_Cursor(50,22);
            printf("Input:  %d",input);
        }
        if(type==CHAR) {
            if(input>=97) input=input-32;
            if( (input>=min_range) && (input<=max_range) ) {
                line_buffer[0]=input;
                data=input;
            } else {
                proper_input_given=FALSE;
                line_buffer[0]=' ';
                data=99;
                Position_Cursor(4,23);
                cprintf("Invalid entry, please enter %c-",min_range);
                cprintf("%c.",max_range);
            }
            Position_Cursor((x_position+1),y_position);
            cprintf("%c",line_buffer[0]);
        }
        if(type==YN) {
            switch (input) {
                case 'Y':
                    line_buffer[0]='Y';
                    data=TRUE;
                    break;
                case 'y':
                    line_buffer[0]='Y';
                    data=TRUE;
                    break;
                case 'N':
                    line_buffer[0]='N';
                    data=FALSE;
                    break;
                case 'n':
                    line_buffer[0]='N';
                    data=FALSE;
                    break;
                default:
                    proper_input_given=FALSE;
                    line_buffer[0]=' ';
                    data=99;
                    Position_Cursor(4,23);
                    cprintf("Invalid entry, please enter Y-N.");
            }
            Position_Cursor((x_position+1),y_position);
            cprintf("%c",line_buffer[0]);
        }
        if( (type==NUM) && (input!='\xff') ) {
            if(input!=8) {
                invalid_input=FALSE;
                if(size_of_field>1) {
                    min_range=0;
                    max_range=9;
                }
                if( (input>='0') && (input<='9') )input=input-48;
                else {
                    if(input<10) input=11;
                }
                if( ( (size_of_field>1) && (input>max_range) ) || (input>9) ) {
                    proper_input_given=FALSE;
                    Position_Cursor(4,23);
                    cprintf("Invalid entry, please enter %d-%d.",min_range,max_range);
                    invalid_input=TRUE;
                }
                if( (size_of_field==1) && ( (input<min_range) || ( (input>max_range) && (input<10) ) ) ) {
                    proper_input_given=FALSE;
                    Position_Cursor(4,23);
                    cprintf("%d is not a choice, please enter ",input);
                    cprintf("%d-%d.",min_range,max_range);
                    invalid_input=TRUE;
                }
                if( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field>1) ) {
                    proper_input_given=FALSE;
                    Position_Cursor(4,23);
                    cprintf("Invalid entry.");
                    invalid_input=TRUE;
                }
                if( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field==1) ) {
                    line_buffer_index=0;
                }
                if(invalid_input==FALSE) {
                    if( (line_buffer_index==1) && (line_buffer[0]=='0') ) {
                        line_buffer[0]=0;
                        line_buffer_index=0;
                    }
                    line_buffer[line_buffer_index]=(input+48);
                    line_buffer_index++;
                }
            } else {
                line_buffer_index--;
                if(line_buffer_index<0) line_buffer_index=0;
                line_buffer[line_buffer_index]=0;
                if(line_buffer_index==0) {
                    line_buffer[0]='0';
                    line_buffer_index=1;
                }
            }
            index=0;
            do {
                Position_Cursor((x_position+1+index),y_position);
                printf(" ");
                index++;
            } while(index<size_of_field);
            index=line_buffer_index;
            do {
                Position_Cursor((x_position+size_of_field-line_buffer_index+index),y_position);
                index--;
                cprintf("%c",line_buffer[index]);
            } while(index>0);
        }
        if( (type==NUMP) && (input!='\xff') ) {
            if(input!=8) {
                invalid_input=FALSE;
                if(size_of_field>1) {
                    min_range=0;
                    max_range=9;
                }
                if( (input=='%') && (percent_entered==FALSE) ) {
                    percent_entered=TRUE;
                    percent_just_entered=TRUE;
                }
                if( (input>='0') && (input<='9') )input=input-48;
                else {
                    if(input<10) input=11;
                }
                if( (percent_entered==FALSE) && (percent_just_entered==FALSE) && ( ( (size_of_field>1) && (input>max_range) ) || (input>9) ) ) {
                    proper_input_given=FALSE;
                    Position_Cursor(4,23);
                    cprintf("Invalid entry, please enter %d-%d.",min_range,max_range);
                    invalid_input=TRUE;
                }
                if( (percent_entered==FALSE) && (size_of_field==1) && ( (input<min_range) || ( (input>max_range) && (input<10) ) ) ) {
                    proper_input_given=FALSE;
                    Position_Cursor(4,23);
                    cprintf("%d is not a choice, please enter ",input);
                    cprintf("%d-%d.",min_range,max_range);
                    invalid_input=TRUE;
                }
                if( ( (percent_entered==TRUE) && (percent_just_entered==FALSE) ) || ( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field>1) ) ) {
                    proper_input_given=FALSE;
                    Position_Cursor(4,23);
                    cprintf("Invalid entry.");
                    invalid_input=TRUE;
                }
                if( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field==1) ) {
                    line_buffer_index=0;
                }
                if(invalid_input==FALSE) {
                    if( (line_buffer_index==1) && (line_buffer[0]=='0') ) {
                        line_buffer[0]=0;
                        line_buffer_index=0;
                    }
                    if(percent_just_entered==TRUE) {
                        percent_just_entered=FALSE;
                        line_buffer[line_buffer_index]='%';
                        line_buffer_index++;
                    } else {
                        line_buffer[line_buffer_index]=(input+48);
                        line_buffer_index++;
                    }
                }
            } else {
                line_buffer_index--;
                if(line_buffer_index<0) line_buffer_index=0;
                line_buffer[line_buffer_index]=0;
                if(line_buffer_index==0) {
                    line_buffer[0]='0';
                    line_buffer_index=1;
                }
                if(percent_entered==TRUE) percent_entered=FALSE;
            }
            index=0;
            do {
                Position_Cursor((x_position+1+index),y_position);
                printf(" ");
                index++;
            } while(index<size_of_field);
            index=line_buffer_index;
            do {
                Position_Cursor((x_position+size_of_field-line_buffer_index+index),y_position);
                index--;
                cprintf("%c",line_buffer[index]);
            } while(index>0);
        }
        if(debug.input_routine==TRUE) {
            Position_Cursor(60,23);
            printf("                ");
            Position_Cursor(60,24);
            printf("                ");
            Position_Cursor(50,23);
            printf("Line Buffer:  %10s",line_buffer);
            Position_Cursor(50,24);
            printf("Line Buffer Index:  %d",line_buffer_index);
            Position_Cursor(75,24);
            if(percent_entered==TRUE) {
                printf("P");
            } else {
                printf("  ");
            }
        }
        if(type!=ESC) {
            Position_Cursor(x_position,y_position);
            cprintf("[");
            Position_Cursor((x_position+size_of_field+1),y_position);
            cprintf("]");
        }
    } while(proper_input_given==FALSE);
    percent_entered++; /* dummy */
    return(data);
}
void Interactive_User_Interface() {
    int menu=MM;
    Create_MBR_If_Not_Present();
    do {
        menu=Standard_Menu(menu);
        if(menu==CPDP) Create_DOS_Partition_Interface(PRIMARY);
        if(menu==CEDP) Create_DOS_Partition_Interface(EXTENDED);
        if(menu==CLDD) {
            if(partition_table[(flags.drive_number-128)].extended_partition_exists==FALSE) {
                Position_Cursor(4,22);
                cprintf("Cannot create Logical DOS Drive without");
                Position_Cursor(4,23);
                cprintf("an Extended DOS Partition on the current drive.");
                Position_Cursor(4,24);
                printf("                                        ");
                Input(0,0,0,ESC,0,0,ESCC,0,0);
                menu=MM;
            } else Create_Logical_Drive_Interface();
        }
        if(menu==SAP) Set_Active_Partition_Interface();
        if(menu==DPDP) Delete_Primary_DOS_Partition_Interface();
        if(menu==DEDP) {
            if(partition_table[(flags.drive_number-128)].extended_partition_exists==FALSE) {
                Position_Cursor(4,22);
                cprintf("No Extended DOS Partition to delete.");
                Position_Cursor(4,24);
                printf("                                        ");
                Input(0,0,0,ESC,0,0,ESCC,0,0);
                menu=MM;
            } else Delete_Extended_DOS_Partition_Interface();
        }
        if(menu==DLDD) {
            if( (partition_table[(flags.drive_number-128)].number_of_logical_drives==0) || (partition_table[(flags.drive_number-128)].extended_partition_exists==FALSE) ) {
                Position_Cursor(4,22);
                cprintf("No Logical DOS Drive(s) to delete.");
                Position_Cursor(4,24);
                printf("                                        ");
                Input(0,0,0,ESC,0,0,ESCC,0,0);
                menu=MM;
            } else Delete_Logical_Drive_Interface();
        }
        if(menu==DNDP) Delete_N_DOS_Partition_Interface();
        if( (menu==DPI) && (flags.extended_options_flag==FALSE) ) Display_Partition_Information();
        if( (menu==DPI) && (flags.extended_options_flag==TRUE) ) Display_Or_Modify_Partition_Information();
        if(menu==CD)  Change_Current_Fixed_Disk_Drive();
        if(menu!=EXIT) {
            if( (menu>0x0f) || (menu==MM) )menu=MM;
            else menu=menu&0xf0;
        }
    } while(menu!=EXIT);
    if(flags.partitions_have_changed==TRUE) {
        Write_Partition_Tables();
        asm{
            mov ah,0
            mov al,3
            int 0x10
        }
        if(flags.reboot==FALSE) {
            Print_Centered(5,"Reboot computer to ensure that changes take effect.",BOLD);
            Position_Cursor(4,10);
            printf("If the computer is not rebooted file system corruption may result.");
            Position_Cursor(4,14);
        } else Reboot_PC();
    } else {
        asm{
            mov ah,0
            mov al,3
            int 0x10
        }
    }
}
void Load_Brief_Partition_Table() {
    int index=0;
    int sub_index=0;
    index=0;
    do {
        sub_index=0;
        do {
            brief_partition_table[index] [sub_index]=0;
            sub_index++;
        } while(sub_index<27);
        index++;
    } while(index<8);
    index=0;
    do {
        sub_index=0;
        do {
            brief_partition_table[index] [sub_index]=partition_table[index].primary_partition_numeric_type[sub_index];
            sub_index++;
        } while(sub_index<4);
        sub_index=0;
        do {
            brief_partition_table[index] [(sub_index+4)]=partition_table[index].logical_drive_numeric_type[sub_index];
            sub_index++;
        } while(sub_index<23);
        index++;
    } while(index<8);
}
void Load_LBA_Into_DAP(unsigned long lba_address) {
    int value1;
    int value2;
    int value3;
    int value4;
    *(unsigned long *)lba_address_l=lba_address;
    value1=disk_address_packet[12];
    value2=disk_address_packet[13];
    value3=disk_address_packet[14];
    value4=disk_address_packet[15];
    disk_address_packet[12]=value4;
    disk_address_packet[13]=value3;
    disk_address_packet[14]=value2;
    disk_address_packet[15]=value1;
}
void Modify_Partition_Type(int partition_number,int type_number) {
    partition_table[(flags.drive_number-128)].primary_partition_numeric_type[partition_number]=type_number;
    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
    flags.partitions_have_changed=TRUE;
}
void Position_Cursor(int column,int row) {
    asm{
        mov ah,0x0f
        int 0x10
        mov ah,0x02
        mov dh,BYTE PTR row
        mov dl,BYTE PTR column
        int 0x10
    }
}
void Print_Centered(int y,char *text,int style) {
    int x=40-strlen(text)/2;
    Position_Cursor(x,y);
    if(style==BOLD) cprintf(text);
    else printf(text);
}
void Process_Fdiskini_File()
{
  char char_number[2];
  char command_buffer[20];
  char home_path[255];
  char line_buffer[256];
  char setting_buffer[20];
  int index=0;
  int command_ok=FALSE;
  int done_looking=FALSE;
  int end_of_file_marker_encountered=FALSE;
  int object_found=FALSE;
  int sub_buffer_index=0;
  long line_counter=1;
  long setting;
  FILE *file;
  debug.all=UNCHANGED;
  debug.create_partition=UNCHANGED;
  debug.determine_free_space=UNCHANGED;
  debug.emulate_disk=UNCHANGED;
  debug.input_routine=UNCHANGED;
  debug.LBA=UNCHANGED;
  debug.path=UNCHANGED;
  debug.write=UNCHANGED;
  flags.extended_options_flag=UNCHANGED;
  flags.monochrome=UNCHANGED;
  flags.label=UNCHANGED;
  flags.reboot=UNCHANGED;
  flags.use_ambr=UNCHANGED;
  flags.version=UNCHANGED;
  strcpy(home_path,path);
  strcat(home_path,"fdisk.ini");
  file=fopen(home_path,"rt");
  if(!file) file=fopen(searchpath("fdisk.ini"),"rt");
  if(file)
    {
    while(fgets(line_buffer,255,file) !=NULL)
      {
      if( (0!=strncmp(line_buffer,";",1)) && (0!=strncmp(line_buffer,"999",3)) && (end_of_file_marker_encountered==FALSE) )
	{
	index=0;
	do
	  {
	  command_buffer[index]=0x00;
	  setting_buffer[index]=0x00;
	  index++;
	  }while(index<20);
	index=0;
	sub_buffer_index=0;
	done_looking=FALSE;
	object_found=FALSE;
	do
	  {
	  if( (line_buffer[index]!='=') && ( (line_buffer[index]>=0x30) && (line_buffer[index]<=0x7a) ) )
	    {
	    object_found=TRUE;
	    command_buffer[sub_buffer_index]=line_buffer[index];
	    sub_buffer_index++;
	    }
	  if( (object_found==TRUE) && ( (line_buffer[index]=='=') || (line_buffer[index]==' ') ) )
	    {
	    done_looking=TRUE;
	    }
	  if( (index==254) || (line_buffer[index]==0x0a) )
	    {
	    printf("Error encountered on line %n of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  index++;
	  }while(done_looking==FALSE);
	sub_buffer_index=0;
	object_found=FALSE;
	done_looking=FALSE;
	do
	  {
	  if( (line_buffer[index]!='=') && ( (line_buffer[index]>=0x30) && (line_buffer[index]<=0x7a) ) )
	    {
	    object_found=TRUE;
	    setting_buffer[sub_buffer_index]=line_buffer[index];
	    sub_buffer_index++;
	    }
	  if( (object_found==TRUE) && (line_buffer[index]==0x0a) )
	    {
	    done_looking=TRUE;
	    }
	  if(index==254)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  index++;
	  }while(done_looking==FALSE);
	command_ok=FALSE;
	if(0==stricmp(command_buffer,"AMBR"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.use_ambr=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.use_ambr=FALSE;
	  if(flags.use_ambr==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"D_ALL"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.all=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.all=FALSE;
	  if(debug.all==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"D_CR_PART"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.create_partition=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.create_partition=FALSE;
	  if(debug.create_partition==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"D_DET_FR_SPC"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.determine_free_space=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.determine_free_space=FALSE;
	  if(debug.determine_free_space==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"D_INPUT"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.input_routine=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.input_routine=FALSE;
	  if(debug.input_routine==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"D_LBA"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.LBA=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.LBA=FALSE;
	  if(debug.LBA==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"D_PATH"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.path=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.path=FALSE;
	  if(debug.path==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"EMULATE_DISK"))
	  {
	  if(0==stricmp(setting_buffer,"OFF")) debug.emulate_disk=0;
	  if(0==stricmp(setting_buffer,"1")) debug.emulate_disk=1;
	  if(0==stricmp(setting_buffer,"2")) debug.emulate_disk=2;
	  if(0==stricmp(setting_buffer,"3")) debug.emulate_disk=3;
	  if(0==stricmp(setting_buffer,"4")) debug.emulate_disk=4;
	  if(0==stricmp(setting_buffer,"5")) debug.emulate_disk=5;
	  if(0==stricmp(setting_buffer,"6")) debug.emulate_disk=6;
	  if(0==stricmp(setting_buffer,"7")) debug.emulate_disk=7;
	  if(0==stricmp(setting_buffer,"8")) debug.emulate_disk=8;
	  if(debug.emulate_disk==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"LABEL"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.label=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.label=FALSE;
	  if(flags.label==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"MONO"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.monochrome=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.monochrome=FALSE;
	  if(flags.monochrome==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"REBOOT"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.reboot=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.reboot=FALSE;
	  if(flags.reboot==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"VERSION"))
	  {
	  if(0==stricmp(setting_buffer,"4")) flags.version=FOUR;
	  if(0==stricmp(setting_buffer,"5")) flags.version=FIVE;
	  if(0==stricmp(setting_buffer,"6")) flags.version=SIX;
	  if(0==stricmp(setting_buffer,"W95")) flags.version=W95;
	  if(0==stricmp(setting_buffer,"W95B")) flags.version=W95B;
	  if(0==stricmp(setting_buffer,"W98")) flags.version=W98;
	  if(flags.version==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"XO"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.extended_options_flag=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.extended_options_flag=FALSE;
	  if(flags.extended_options_flag==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(0==stricmp(command_buffer,"WRITE"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.write=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.write=FALSE;
	  if(debug.write==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
	if(command_ok==FALSE)
	  {
	  printf("Error encountered on line %d of the \"fdisk.ini\" file...Operation Terminated.\n",line_counter);
	  exit(3);
	  }
	}
      if(0==strncmp(line_buffer,"999",3)) end_of_file_marker_encountered=TRUE;
      line_counter++;
      }
    fclose(file);
    }
  if(debug.all==UNCHANGED) debug.all=FALSE;
  if(debug.create_partition==UNCHANGED) debug.create_partition=FALSE;
  if(debug.determine_free_space==UNCHANGED) debug.determine_free_space=FALSE;
  if(debug.emulate_disk==UNCHANGED) debug.emulate_disk=0;
  if(debug.input_routine==UNCHANGED) debug.input_routine=FALSE;
  if(debug.LBA==UNCHANGED) debug.LBA=FALSE;
  if(debug.path==UNCHANGED) debug.path=FALSE;
  if(debug.write==UNCHANGED) debug.write=TRUE;
  if(flags.extended_options_flag==UNCHANGED) flags.extended_options_flag=FALSE;
  if(flags.label==UNCHANGED) flags.label=FALSE;
  if(flags.monochrome==UNCHANGED) flags.monochrome=FALSE;
  if(flags.reboot==UNCHANGED) flags.reboot=FALSE;
  if(flags.use_ambr==UNCHANGED) flags.use_ambr=FALSE;
  if(flags.version==UNCHANGED) flags.version=SIX;
  if(debug.all==TRUE)
    {
    debug.create_partition=TRUE;
    debug.determine_free_space=TRUE;
    debug.input_routine=TRUE;
    debug.LBA=TRUE;
    debug.path=TRUE;
    }
  if(debug.emulate_disk!=0) debug.write=FALSE;
}
int Read_Partition_Tables() {
    int drive=0;
    int error_code=0;
    int index;
    int sub_index;
    long entry_offset;
    int physical_drive=0x80;
    unsigned long total_cylinders=0;
    unsigned long total_heads=0;
    unsigned long total_sectors=0;
    do {
        total_cylinders=0;
        total_heads=0;
        total_sectors=0;
        asm{
            mov ah, 0x08
            mov dl, BYTE PTR physical_drive
            int 0x13
            mov bl,cl
            and bl,00111111B
            mov BYTE PTR error_code, ah
            mov BYTE PTR total_sectors, bl
            mov bl,cl
            mov cl,ch
            shr bl,1
            shr bl,1
            shr bl,1
            shr bl,1
            shr bl,1
            shr bl,1
            mov ch,bl
            mov WORD PTR total_cylinders, cx
            mov BYTE PTR total_heads, dh
        }
        partition_table[drive].total_cylinders=total_cylinders;
        partition_table[drive].total_heads=total_heads;
        partition_table[drive].total_sectors=total_sectors;
        partition_table[drive].number_of_logical_drives=0;
        if( (error_code==0) && (debug.emulate_disk!=(drive+1) ) ) {
            partition_table[drive].total_hard_disk_size_in_logical_sectors=(partition_table[drive].total_cylinders+1)*(partition_table[drive].total_heads+1)*partition_table[drive].total_sectors;
            partition_table[drive].total_hard_disk_size_in_MB=partition_table[drive].total_hard_disk_size_in_logical_sectors/2048;
        } else {
            if(debug.emulate_disk==(drive+1) ) {
                partition_table[drive].total_cylinders=EMULATED_CYLINDERS;
                partition_table[drive].total_heads=EMULATED_HEADS;
                partition_table[drive].total_sectors=EMULATED_SECTORS;
                partition_table[drive].total_hard_disk_size_in_logical_sectors=(partition_table[drive].total_cylinders+1)*(partition_table[drive].total_heads+1)*partition_table[drive].total_sectors;
                partition_table[drive].total_hard_disk_size_in_MB=partition_table[drive].total_hard_disk_size_in_logical_sectors/2048;
                flags.maximum_drive_number=drive+128;
            } else {
                if(drive==0) {
                    cprintf("\n    No fixed disks present.\n");
                    exit(6);
                }
                partition_table[drive].total_cylinders=0;
                partition_table[drive].total_heads=0;
                partition_table[drive].total_sectors=0;
            }
        }
        partition_table[drive].primary_partition_largest_free_space=0;
        partition_table[drive].pp_largest_free_space_starting_cylinder=0;
        partition_table[drive].pp_largest_free_space_starting_head=0;
        partition_table[drive].pp_largest_free_space_starting_sector=0;
        partition_table[drive].pp_largest_free_space_ending_cylinder=0;
        index=0;
        do {
            partition_table[drive].active_status[index]=0;
            partition_table[drive].primary_partition_numeric_type[index]=0;
            partition_table[drive].primary_partition_starting_cylinder[index]=0;
            partition_table[drive].primary_partition_starting_head[index]=0;
            partition_table[drive].primary_partition_starting_sector[index]=0;
            partition_table[drive].primary_partition_ending_cylinder[index]=0;
            partition_table[drive].primary_partition_ending_head[index]=0;
            partition_table[drive].primary_partition_ending_sector[index]=0;
            partition_table[drive].primary_partition_relative_sectors[index]=0;
            partition_table[drive].primary_partition_number_of_sectors[index]=0;
            partition_table[drive].primary_partition_size_in_MB[index]=0;
            partition_table[drive].primary_partition_physical_order[index]=index;
            index++;
        } while(index<4);
        partition_table[drive].extended_partition_exists=FALSE;
        partition_table[drive].extended_partition_size_in_MB=0;
        partition_table[drive].extended_partition_number_of_sectors=0;
        partition_table[drive].extended_partition_largest_free_space=0;
        partition_table[drive].logical_drive_free_space_starting_cylinder=0;
        partition_table[drive].logical_drive_free_space_ending_cylinder=0;
        partition_table[drive].logical_drive_largest_free_space_location=0;
        partition_table[drive].number_of_extended_partition=UNUSED;
        partition_table[drive].number_of_logical_drives=0;
        index=0;
        do {
            partition_table[drive].logical_drive_numeric_type[index]=0;
            partition_table[drive].logical_drive_starting_cylinder[index]=0;
            partition_table[drive].logical_drive_starting_head[index]=0;
            partition_table[drive].logical_drive_starting_sector[index]=0;
            partition_table[drive].logical_drive_ending_cylinder[index]=0;
            partition_table[drive].logical_drive_ending_head[index]=0;
            partition_table[drive].logical_drive_ending_sector[index]=0;
            partition_table[drive].logical_drive_relative_sectors[index]=0;
            partition_table[drive].logical_drive_number_of_sectors[index]=0;
            partition_table[drive].logical_drive_size_in_MB[index]=0;
            partition_table[drive].next_extended_exists[index]=FALSE;
            partition_table[drive].next_extended_numeric_type[index]=0;
            partition_table[drive].next_extended_starting_cylinder[index]=0;
            partition_table[drive].next_extended_starting_head[index]=0;
            partition_table[drive].next_extended_starting_sector[index]=0;
            partition_table[drive].next_extended_ending_cylinder[index]=0;
            partition_table[drive].next_extended_ending_head[index]=0;
            partition_table[drive].next_extended_ending_sector[index]=0;
            partition_table[drive].next_extended_relative_sectors[index]=0;
            partition_table[drive].next_extended_number_of_sectors[index]=0;
            index++;
        } while(index<24);
        if(error_code==0) {
            error_code=Read_Physical_Sector(physical_drive,0,0,1);
            if(error_code!=0) return(error_code);
            flags.maximum_drive_number=drive+128;
            index=0;
            do {
                entry_offset=0x1be+(index*16);
                partition_table[drive].active_status[index]=sector_buffer[(entry_offset+0x00)];
                partition_table[drive].primary_partition_numeric_type[index]=sector_buffer[(entry_offset+0x04)];
                partition_table[drive].primary_partition_starting_cylinder[index]=Extract_Cylinder(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
                partition_table[drive].primary_partition_starting_head[index]=sector_buffer[(entry_offset+0x01)];
                partition_table[drive].primary_partition_starting_sector[index]=Extract_Sector(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
                partition_table[drive].primary_partition_ending_cylinder[index]=Extract_Cylinder(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
                partition_table[drive].primary_partition_ending_head[index]=sector_buffer[(entry_offset+0x05)];
                partition_table[drive].primary_partition_ending_sector[index]=Extract_Sector(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
                partition_table[drive].primary_partition_relative_sectors[index]=Decimal_Number(sector_buffer[(entry_offset+0x08)],sector_buffer[(entry_offset+0x09)],sector_buffer[(entry_offset+0x0a)],sector_buffer[(entry_offset+0x0b)]);
                partition_table[drive].primary_partition_number_of_sectors[index]=Decimal_Number(sector_buffer[(entry_offset+0x0c)],sector_buffer[(entry_offset+0x0d)],sector_buffer[(entry_offset+0x0e)],sector_buffer[(entry_offset+0x0f)]);
                partition_table[drive].primary_partition_size_in_MB[index]=(partition_table[drive].primary_partition_number_of_sectors[index]/2048);
                if(partition_table[drive].primary_partition_numeric_type[index]==0x05) {
                    partition_table[drive].extended_partition_exists=TRUE;
                    partition_table[drive].number_of_extended_partition=index;
                    partition_table[drive].extended_partition_number_of_sectors=partition_table[drive].primary_partition_number_of_sectors[index];
                    partition_table[drive].extended_partition_size_in_MB=partition_table[drive].primary_partition_size_in_MB[index];
                }
                index++;
            } while(index<4);
            if(partition_table[drive].extended_partition_exists==TRUE) {
                error_code=Read_Physical_Sector(physical_drive,partition_table[drive].primary_partition_starting_cylinder[partition_table[drive].number_of_extended_partition],partition_table[drive].primary_partition_starting_head[partition_table[drive].number_of_extended_partition],partition_table[drive].primary_partition_starting_sector[partition_table[drive].number_of_extended_partition]);
                if(error_code!=0) return(error_code);
                if( (sector_buffer[0x1fe]==0x55) && (sector_buffer[0x1ff]==0xaa) ) {
                    index=0;
                    do {
                        entry_offset=0x1be;
                        if(sector_buffer[(entry_offset+0x04)]>0) partition_table[drive].number_of_logical_drives++;
                        partition_table[drive].logical_drive_numeric_type[index]=sector_buffer[(entry_offset+0x04)];
                        partition_table[drive].logical_drive_starting_cylinder[index]=Extract_Cylinder(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
                        partition_table[drive].logical_drive_starting_head[index]=sector_buffer[(entry_offset+0x01)];
                        partition_table[drive].logical_drive_starting_sector[index]=Extract_Sector(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
                        partition_table[drive].logical_drive_ending_cylinder[index]=Extract_Cylinder(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
                        partition_table[drive].logical_drive_ending_head[index]=sector_buffer[(entry_offset+0x05)];
                        partition_table[drive].logical_drive_ending_sector[index]=Extract_Sector(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
                        partition_table[drive].logical_drive_relative_sectors[index]=Decimal_Number(sector_buffer[(entry_offset+0x08)],sector_buffer[(entry_offset+0x09)],sector_buffer[(entry_offset+0x0a)],sector_buffer[(entry_offset+0x0b)]);
                        partition_table[drive].logical_drive_number_of_sectors[index]=Decimal_Number(sector_buffer[(entry_offset+0x0c)],sector_buffer[(entry_offset+0x0d)],sector_buffer[(entry_offset+0x0e)],sector_buffer[(entry_offset+0x0f)]);
                        partition_table[drive].logical_drive_size_in_MB[index]=partition_table[drive].logical_drive_number_of_sectors[index]/2048;
                        entry_offset=entry_offset+16;
                        if(sector_buffer[(entry_offset+0x04)]==0x05) {
                            partition_table[drive].next_extended_exists[index]=TRUE;
                            partition_table[drive].next_extended_numeric_type[index]=sector_buffer[(entry_offset+0x04)];
                            partition_table[drive].next_extended_starting_cylinder[index]=Extract_Cylinder(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
                            partition_table[drive].next_extended_starting_head[index]=sector_buffer[(entry_offset+0x01)];
                            partition_table[drive].next_extended_starting_sector[index]=Extract_Sector(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
                            partition_table[drive].next_extended_ending_cylinder[index]=Extract_Cylinder(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
                            partition_table[drive].next_extended_ending_head[index]=sector_buffer[(entry_offset+0x05)];
                            partition_table[drive].next_extended_ending_sector[index]=Extract_Sector(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
                            partition_table[drive].next_extended_relative_sectors[index]=Decimal_Number(sector_buffer[(entry_offset+0x08)],sector_buffer[(entry_offset+0x09)],sector_buffer[(entry_offset+0x0a)],sector_buffer[(entry_offset+0x0b)]);
                            partition_table[drive].next_extended_number_of_sectors[index]=Decimal_Number(sector_buffer[(entry_offset+0x0c)],sector_buffer[(entry_offset+0x0d)],sector_buffer[(entry_offset+0x0e)],sector_buffer[(entry_offset+0x0f)]);
                            error_code=Read_Physical_Sector(physical_drive,partition_table[drive].next_extended_starting_cylinder[index],partition_table[drive].next_extended_starting_head[index],partition_table[drive].next_extended_starting_sector[index]);
                            if(error_code!=0) return(error_code);
                        } else index=24;
                        index++;
                    } while(index<24);
                }
            }
        }
        drive++;
        physical_drive=drive+0x80;
    } while(drive<8);
    Determine_Drive_Letters();
    Get_Partition_Information();
    return(0);
}
void Pause() {
    printf("\nPress any key to continue.\n");
    asm{
        mov ah,7
        int 0x21
    }
}
int Read_Physical_Sector(int drive, long cylinder, long head, long sector) {
    int error_code;
    error_code=biosdisk(2, drive, head, cylinder, sector, 1, sector_buffer);
    return(error_code);
}
void Reboot_PC() {
    void ((far * fp) ())=(void(far*) () ) ((0xffffL<<16) | 0x0000L);
    *(int far *) ((0x0040L << 16) | 0x0072)=0;
    fp();
}
void Remove_MBR() {
    long index=0;
    Read_Physical_Sector((flags.drive_number),0,0,1);
    do {
        sector_buffer[index]=0x00;
        index++;
    } while(index<0x1be);
    Write_Physical_Sector((flags.drive_number),0,0,1);
}
void Set_Active_Partition(int partition_number) {
    int index=0;
    do {
        if(index==partition_number) partition_table[(flags.drive_number-128)].active_status[index]=0x80;
        else partition_table[(flags.drive_number-128)].active_status[index]=0x00;
        index++;
    } while(index<4);
    partition_table[(flags.drive_number-128)].partition_values_changed=TRUE;
    flags.partitions_have_changed=TRUE;
}
int Set_Active_Partition_Interface() {
    int correct_input=FALSE;
    int index=0;
    int input;
    int available_partition_counter=0;
    int first_available_partition_active=FALSE;
    int only_active_partition_active=FALSE;
    int partition_settable[4];
    do {
        partition_settable[index]=FALSE;
        if( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==1) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==4) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==6) || ( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[index]==0x0b) && ( (flags.version==W95B) || (flags.version==W98) ) ) ) {
            available_partition_counter++;
            if( (available_partition_counter==1) && (partition_table[(flags.drive_number-128)].active_status[index]==0x80) )first_available_partition_active=TRUE;
            partition_settable[index]=TRUE;
        }
        index++;
    } while(index<=3);
    if( (available_partition_counter==1) && (first_available_partition_active==TRUE) ) only_active_partition_active=TRUE;
    Clear_Screen();
    Print_Centered(4,"Set Active Partition",BOLD);
    Display_Primary_Partition_Information_SS();
    if(available_partition_counter==0) {
        Position_Cursor(4,22);
        cprintf("No partitions to make active.");
        Input(0,0,0,ESC,0,0,ESCC,0,0);
    }
    if( (only_active_partition_active==FALSE) && (available_partition_counter>0) ) {
        Position_Cursor(4,16);
        printf("Enter the number of the partition you want to make active...........: ");
        do {
            flags.esc=FALSE;
            input=Input(1,70,16,NUM,1,4,ESCR,-1,0);
            if(flags.esc==TRUE) return(1);
            if(partition_settable[(input-1)]==TRUE) correct_input=TRUE;
            else {
                Position_Cursor(4,23);
                cprintf("%d is not a choice. Please enter a valid choice.",input);
            }
        } while(correct_input==FALSE);
        Set_Active_Partition(input-1);
        Clear_Screen();
        Print_Centered(4,"Set Active Partition",BOLD);
        Display_Primary_Partition_Information_SS();
        Input(0,0,0,ESC,0,0,ESCC,0,0);
    }
    if(only_active_partition_active==TRUE) {
        Position_Cursor(4,22);
        cprintf("The only startable partition on Drive %d is already set active.",(flags.drive_number-127));
        Input(0,0,0,ESC,0,0,ESCC,0,0);
    }
    return(0);
}
int Standard_Menu(int menu) {
    int display_menu=TRUE;
    int result;
    int maximum_number_of_options=0;
    int input;
    char program_name[60]="";
    char program_description[60]="";
    char copyright[60]="";
    char title[60]="";
    char option_1[60]="";
    char option_2[60]="";
    char option_3[60]="";
    char option_4[60]="";
    char option_5[60]="Change current fixed disk drive";
    do {
        if(menu==MM) {
            maximum_number_of_options=4;
            strcpy(title,"FDISK Options");
            strcpy(option_1,"Create DOS partition or Logical DOS Drive");
            strcpy(option_2,"Set Active partition");
            strcpy(option_3,"Delete partition or Logical DOS Drive");
            if(flags.extended_options_flag==FALSE) strcpy(option_4,"Display partition information");
            else strcpy(option_4,"Display/Modify partition information");
        }
        if(menu==CP) {
            maximum_number_of_options=3;
            strcpy(title,"Create DOS Partition or Logical DOS Drive");
            strcpy(option_1,"Create Primary DOS Partition");
            strcpy(option_2,"Create Extended DOS Partition");
            strcpy(option_3,"Create Logical DOS Drive(s) in the Extended DOS Partition");
            strcpy(option_4,"");
        }
        if(menu==DP) {
            maximum_number_of_options=4;
            strcpy(title,"Delete DOS Partition or Logical DOS Drive");
            strcpy(option_1,"Delete Primary DOS Partition");
            strcpy(option_2,"Delete Extended DOS Partition");
            strcpy(option_3,"Delete Logical DOS Drive(s) in the Extended DOS Partition");
            strcpy(option_4,"Delete Non-DOS Partition");
        }
        Clear_Screen();
        if(menu!=MM) flags.display_name_description_copyright=FALSE;
        if(flags.display_name_description_copyright==TRUE) {
            Print_Centered(0,program_name,STANDARD);
            Print_Centered(1,program_description,STANDARD);
            Print_Centered(2,copyright,STANDARD);
        }
        Print_Centered(4,title,BOLD);
        Position_Cursor(4,6);
        printf("Current fixed disk drive: ");
        cprintf("%d",(flags.drive_number-127));
        Position_Cursor(4,8);
        printf("Choose one of the following:");
        Position_Cursor(4,10);
        cprintf("1.  ");
        printf("%s",option_1);
        if(maximum_number_of_options>1) {
            Position_Cursor(4,11);
            cprintf("2.  ");
            printf("%s",option_2);
        }
        if(maximum_number_of_options>2) {
            Position_Cursor(4,12);
            cprintf("3.  ");
            printf("%s",option_3);
        }
        if(maximum_number_of_options>3) {
            Position_Cursor(4,13);
            cprintf("4.  ");
            printf("%s",option_4);
        }
        if( (menu==MM) && (flags.more_than_one_drive==TRUE) ) {
            maximum_number_of_options=5;
            Position_Cursor(4,14);
            cprintf("5.  ");
            printf("%s",option_5);
        }
        if( ( (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[0]>0) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[1]>0) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[2]>0) || (partition_table[(flags.drive_number-128)].primary_partition_numeric_type[3]>0) ) && (flags.drive_number==0x80) && (menu==MM) && (partition_table[flags.drive_number-128].active_status[0]==0) && (partition_table[flags.drive_number-128].active_status[1]==0) && (partition_table[flags.drive_number-128].active_status[2]==0) && (partition_table[flags.drive_number-128].active_status[3]==0) ) {
            Position_Cursor(4,21);
            cprintf("WARNING! ");
            printf("No partitions are set active - disk 1 is not startable unless");
            Position_Cursor(4,22);
            printf("a partition is set active");
        }
        Position_Cursor(4,17);
        printf("Enter choice: ");
        if(menu==MM) input=Input(1,19,17,NUM,1,maximum_number_of_options,ESCE,1,0);
        else input=Input(1,19,17,NUM,1,maximum_number_of_options,ESCR,-1,0);
        if(input!=0) {
            if(menu==MM) menu=input<<4;
            else menu=menu|input;
        } else {
            if(menu==MM) {
                menu=EXIT;
            } else {
                if(menu>0x0f) menu=MM;
                else menu=menu&0xf0;
            }
        }
        if( (menu==MM) || (menu==CP) || (menu==DP) ) display_menu=TRUE;
        else display_menu=FALSE;
    } while(display_menu==TRUE);
    flags.display_name_description_copyright=FALSE;
    return(menu);
}
void Save_MBR() {
    long index=0;
    FILE *file_pointer;
    Read_Physical_Sector(flags.drive_number,0,0,1);
    file_pointer = fopen("boot.mbr","wb");
    if(!file_pointer) {
        printf("\nError opening or creating \"BOOT.MBR\" for writing...Operation Terminated.\n");
        exit(8);
    }
    do {
        fputc(sector_buffer[index],file_pointer);
        index++;
    } while(index<0x1be);
    do {
        fputc(0,file_pointer);
        index++;
    } while(index<512);
    fclose(file_pointer);
}
int Write_Partition_Tables() {
    int error_code;
    int index;
    int drive_index=0;
    long entry_offset;
    unsigned long high;
    unsigned long low;
    unsigned long temp;
    long extended_cylinder;
    long extended_head;
    long extended_sector;
    long next_extended_cylinder;
    long next_extended_head;
    long next_extended_sector;
    do {
        if(partition_table[drive_index].partition_values_changed==TRUE) {
            index=0;
            Clear_Sector_Buffer();
            if(debug.write==TRUE) error_code=Read_Physical_Sector((drive_index+128),0,0,1);
            else error_code=0;
            if(error_code!=0) return(error_code);
            Clear_Partition_Table_Area_Of_Sector_Buffer();
            do {
                if(partition_table[drive_index].primary_partition_numeric_type[index]==0x05) {
                    extended_cylinder=partition_table[drive_index].primary_partition_starting_cylinder[index];
                    extended_head=partition_table[drive_index].primary_partition_starting_head[index];
                    extended_sector=partition_table[drive_index].primary_partition_starting_sector[index];
                }
                entry_offset=0x1be+(index*16);
                sector_buffer[(entry_offset+0x00)]=partition_table[drive_index].active_status[index];
                sector_buffer[(entry_offset+0x01)]=partition_table[drive_index].primary_partition_starting_head[index];
                Convert_Long_To_Integer(Combine_Cylinder_and_Sector(partition_table[drive_index].primary_partition_starting_cylinder[index],partition_table[drive_index].primary_partition_starting_sector[index]));
                sector_buffer[(entry_offset+0x02)]=integer1;
                sector_buffer[(entry_offset+0x03)]=integer2;
                sector_buffer[(entry_offset+0x04)]=partition_table[drive_index].primary_partition_numeric_type[index];
                sector_buffer[(entry_offset+0x05)]=partition_table[drive_index].primary_partition_ending_head[index];
                Convert_Long_To_Integer(Combine_Cylinder_and_Sector(partition_table[drive_index].primary_partition_ending_cylinder[index],partition_table[drive_index].primary_partition_ending_sector[index]));
                sector_buffer[(entry_offset+0x06)]=integer1;
                sector_buffer[(entry_offset+0x07)]=integer2;
                high = partition_table[drive_index].primary_partition_relative_sectors[index] >> 16;
                temp=high << 16;
                if(partition_table[drive_index].primary_partition_relative_sectors[index] > 0xffff) {
                    low = partition_table[drive_index].primary_partition_relative_sectors[index] - temp;
                } else {
                    low = partition_table[drive_index].primary_partition_relative_sectors[index];
                }
                Convert_Long_To_Integer(low);
                sector_buffer[entry_offset+0x08]=integer1;
                sector_buffer[entry_offset+0x09]=integer2;
                Convert_Long_To_Integer(high);
                sector_buffer[entry_offset+0x0a]=integer1;
                sector_buffer[entry_offset+0x0b]=integer2;
                high = partition_table[drive_index].primary_partition_number_of_sectors[index] >> 16;
                temp=high << 16;
                if(partition_table[drive_index].primary_partition_number_of_sectors[index] > 0xffff) {
                    low = partition_table[drive_index].primary_partition_number_of_sectors[index] - temp;
                } else {
                    low = partition_table[drive_index].primary_partition_number_of_sectors[index];
                }
                Convert_Long_To_Integer(low);
                sector_buffer[entry_offset+0x0c]=integer1;
                sector_buffer[entry_offset+0x0d]=integer2;
                Convert_Long_To_Integer(high);
                sector_buffer[entry_offset+0x0e]=integer1;
                sector_buffer[entry_offset+0x0f]=integer2;
                index++;
            } while(index<4);
            sector_buffer[0x1fe]=0x55;
            sector_buffer[0x1ff]=0xaa;
            error_code=Write_Physical_Sector((drive_index+128),0,0,1);
            if(error_code>0) return(error_code);
            if(partition_table[drive_index].extended_partition_exists==TRUE) {
                index=0;
                do {
                    Clear_Sector_Buffer();
                    entry_offset=0x1be;
                    sector_buffer[0x1fe]=0x55;
                    sector_buffer[0x1ff]=0xaa;
                    sector_buffer[(entry_offset+0x01)]=partition_table[drive_index].logical_drive_starting_head[index];
                    Convert_Long_To_Integer(Combine_Cylinder_and_Sector(partition_table[drive_index].logical_drive_starting_cylinder[index],partition_table[drive_index].logical_drive_starting_sector[index]));
                    sector_buffer[(entry_offset+0x02)]=integer1;
                    sector_buffer[(entry_offset+0x03)]=integer2;
                    sector_buffer[(entry_offset+0x04)]=partition_table[drive_index].logical_drive_numeric_type[index];
                    sector_buffer[(entry_offset+0x05)]=partition_table[drive_index].logical_drive_ending_head[index];
                    Convert_Long_To_Integer(Combine_Cylinder_and_Sector(partition_table[drive_index].logical_drive_ending_cylinder[index],partition_table[drive_index].logical_drive_ending_sector[index]));
                    sector_buffer[(entry_offset+0x06)]=integer1;
                    sector_buffer[(entry_offset+0x07)]=integer2;
                    high = partition_table[drive_index].logical_drive_relative_sectors[index] >> 16;
                    temp=high << 16;
                    if(partition_table[drive_index].logical_drive_relative_sectors[index] > 0xffff) {
                        low = partition_table[drive_index].logical_drive_relative_sectors[index] - temp;
                    } else {
                        low = partition_table[drive_index].logical_drive_relative_sectors[index];
                    }
                    Convert_Long_To_Integer(low);
                    sector_buffer[entry_offset+0x08]=integer1;
                    sector_buffer[entry_offset+0x09]=integer2;
                    Convert_Long_To_Integer(high);
                    sector_buffer[entry_offset+0x0a]=integer1;
                    sector_buffer[entry_offset+0x0b]=integer2;
                    high = partition_table[drive_index].logical_drive_number_of_sectors[index] >> 16;
                    temp=high << 16;
                    if(partition_table[drive_index].logical_drive_number_of_sectors[index] > 0xffff) {
                        low = partition_table[drive_index].logical_drive_number_of_sectors[index] - temp;
                    } else {
                        low = partition_table[drive_index].logical_drive_number_of_sectors[index];
                    }
                    Convert_Long_To_Integer(low);
                    sector_buffer[entry_offset+0x0c]=integer1;
                    sector_buffer[entry_offset+0x0d]=integer2;
                    Convert_Long_To_Integer(high);
                    sector_buffer[entry_offset+0x0e]=integer1;
                    sector_buffer[entry_offset+0x0f]=integer2;
                    if(partition_table[drive_index].next_extended_exists[index]==TRUE) {
                        next_extended_cylinder=partition_table[drive_index].next_extended_starting_cylinder[index];
                        next_extended_head=partition_table[drive_index].next_extended_starting_head[index];
                        next_extended_sector=partition_table[drive_index].next_extended_starting_sector[index];
                        entry_offset=entry_offset+16;
                        sector_buffer[(entry_offset+0x01)]=partition_table[drive_index].next_extended_starting_head[index];
                        Convert_Long_To_Integer(Combine_Cylinder_and_Sector(partition_table[drive_index].next_extended_starting_cylinder[index],partition_table[drive_index].next_extended_starting_sector[index]));
                        sector_buffer[(entry_offset+0x02)]=integer1;
                        sector_buffer[(entry_offset+0x03)]=integer2;
                        sector_buffer[(entry_offset+0x04)]=partition_table[drive_index].next_extended_numeric_type[index];
                        sector_buffer[(entry_offset+0x05)]=partition_table[drive_index].next_extended_ending_head[index];
                        Convert_Long_To_Integer(Combine_Cylinder_and_Sector(partition_table[drive_index].next_extended_ending_cylinder[index],partition_table[drive_index].next_extended_ending_sector[index]));
                        sector_buffer[(entry_offset+0x06)]=integer1;
                        sector_buffer[(entry_offset+0x07)]=integer2;
                        high = partition_table[drive_index].next_extended_relative_sectors[index] >> 16;
                        temp=high << 16;
                        if(partition_table[drive_index].next_extended_relative_sectors[index] > 0xffff) {
                            low = partition_table[drive_index].next_extended_relative_sectors[index] - temp;
                        } else {
                            low = partition_table[drive_index].next_extended_relative_sectors[index];
                        }
                        Convert_Long_To_Integer(low);
                        sector_buffer[entry_offset+0x08]=integer1;
                        sector_buffer[entry_offset+0x09]=integer2;
                        Convert_Long_To_Integer(high);
                        sector_buffer[entry_offset+0x0a]=integer1;
                        sector_buffer[entry_offset+0x0b]=integer2;
                        high = partition_table[drive_index].next_extended_number_of_sectors[index] >> 16;
                        temp=high << 16;
                        if(partition_table[drive_index].next_extended_number_of_sectors[index] > 0xffff) {
                            low = partition_table[drive_index].next_extended_number_of_sectors[index] = temp;
                        } else {
                            low = partition_table[drive_index].next_extended_number_of_sectors[index];
                        }
                        Convert_Long_To_Integer(low);
                        sector_buffer[entry_offset+0x0c]=integer1;
                        sector_buffer[entry_offset+0x0d]=integer2;
                        Convert_Long_To_Integer(high);
                        sector_buffer[entry_offset+0x0e]=integer1;
                        sector_buffer[entry_offset+0x0f]=integer2;
                        error_code=Write_Physical_Sector((drive_index+128),extended_cylinder,extended_head,extended_sector);
                        if(error_code!=0) return(error_code);
                        extended_cylinder=next_extended_cylinder;
                        extended_head=next_extended_head;
                        extended_sector=next_extended_sector;
                    } else {
                        error_code=Write_Physical_Sector((drive_index+128),extended_cylinder,extended_head,extended_sector);
                        if(error_code!=0) return(error_code);
                        index=24;
                    }
                    index++;
                } while(index<24);
            }
        }
        drive_index++;
    } while(drive_index<7);
    return(0);
}
int Write_Physical_Sector(int drive, long cylinder, long head, long sector) {
    int current_line=0;
    int error_code;
    long index=0;
    long offset=0x1be;
    if(debug.write==TRUE) {
        error_code=biosdisk(3, drive, head, cylinder, sector, 1, sector_buffer);
    } else {
        Clear_Screen();
        Print_Centered(4,"Write Physical Sector Debug",BOLD);
        Position_Cursor(4,6);
        printf("Note:  WRITE=OFF is set or an emulated disk is in existence...no");
        Position_Cursor(4,7);
        printf("       changes will be made.  Please check the \"fdisk.ini\" file");
        Position_Cursor(4,8);
        printf("       for details.");
        Position_Cursor(4,10);
        printf("Information passed to this function:");
        Position_Cursor(50,11);
        printf("Drive:     0x%X",drive);
        Position_Cursor(50,12);
        printf("Cylinder:  %d",cylinder);
        Position_Cursor(50,13);
        printf("Head:      %d",head);
        Position_Cursor(50,14);
        printf("Sector:    %d",sector);
        Position_Cursor(4,16);
        printf("Contents of partition table area in sector_buffer[]:");
        do {
            index=0;
            Position_Cursor(4,(current_line+18));
            printf("%d:  ",(current_line+1));
            do {
                printf("%02X ",sector_buffer[(index+offset)]);
                index++;
            } while(index<16);
            current_line++;
            offset=offset+16;
        } while(offset<(0x1be+64));
        Position_Cursor(4,23);
        printf("Press any key to continue.");
        asm{
            mov ah,7
            int 0x21
        }
        error_code=0;
    }
    return(error_code);
}
void main(int argc, char *argv[]) {
    int index;
    int location;
    index=strlen(argv[0]);
    location=0;
    do {
        if(argv[0] [index]=='\\') {
            location=index+1;
            index=-1;
        }
        index--;
    } while(index>=0);
    index=location;
    do {
        filename[index-location]=argv[0] [index];
        index++;
    } while(index<=(strlen(argv[0])) );
    index=0;
    do {
        if(filename[index]=='.') filename[index]=0;
        index++;
    } while(index<12);
    if(location>0) {
        index=0;
        do {
            path[index]=argv[0] [index];
            index++;
        } while(index<location);
        path[index]=0;
    } else path[0]=0;
    if(debug.path==TRUE) {
        printf("\nThe PATH to \"%s\" is:  ",filename);
        printf("\"%s\"\n\n",path);
        Pause();
    }
    Initialization();
    if(argc==1) {
        if( (flags.version==W95B) || (flags.version==W98) ) Ask_User_About_FAT32_Support();
        Interactive_User_Interface();
        exit(0);
    }
    if(argc>=2) {
        if(0==stricmp("/ACTOK",argv[1])) {
            if( (flags.version==W95B) || (flags.version==W98) ) Ask_User_About_FAT32_Support();
            Interactive_User_Interface();
            exit(0);
        }
        if( (0==strncmp("/EXT",argv[1],3) ) || (0==strncmp("/ext",argv[1],3) ) ) {
            char char_size[4];
            long size=0;
            char_size[0]=argv[1] [5];
            char_size[1]=argv[1] [6];
            char_size[2]=argv[1] [7];
            char_size[3]=argv[1] [8];
            flags.drive_number=((argv[2] [0])-47)+127;
            size=atol(char_size);
            if( (flags.drive_number<128) || (flags.drive_number>flags.maximum_drive_number) ) {
                printf("\nInvalid drive designation...Operation Terminated.\n");
                exit(5);
            }
            Determine_Free_Space();
            Create_Primary_Partition(5,size);
            Write_Partition_Tables();
            exit(0);
        }
        if(0==stricmp("/FPRMT",argv[1])) {
            printf("\nThe function selected has not yet been implemented...Operation Terminated.\n");
            exit(10);
        }
        if( (0==strncmp("/PRI",argv[1],3) ) || (0==strncmp("/pri",argv[1],3) ) ) {
            int numeric_type;
            char char_size[4];
            long size=0;
            char_size[0]=argv[1] [5];
            char_size[1]=argv[1] [6];
            char_size[2]=argv[1] [7];
            char_size[3]=argv[1] [8];
            flags.drive_number=((argv[2] [0])-47)+127;
            size=atol(char_size);
            if( (flags.drive_number<128) || (flags.drive_number>flags.maximum_drive_number) ) {
                printf("\nInvalid drive designation...Operation Terminated.\n");
                exit(5);
            }
            if(size<=16) numeric_type=1;
            if( (size>16) && (size<=32) ) numeric_type=4;
            if(size>32) numeric_type=6;
            if( (size>512) && ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==FALSE) && (flags.extended_int_13==TRUE) ) numeric_type=0x0e;
            if( (size>512) && ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==TRUE) && (flags.extended_int_13==TRUE) ) numeric_type=0x0b;
            Determine_Free_Space();
            Set_Active_Partition(Create_Primary_Partition(numeric_type,size));
            Write_Partition_Tables();
            exit(0);
        }
        if( (0==strncmp("/PRIO",argv[1],4) ) || (0==strncmp("/prio",argv[1],4) ) ) {
            printf("\nThe function selected has not yet been implemented...Operation Terminated.\n");
            exit(10);
        }
        if(0==stricmp("/STATUS",argv[1])) {
            flags.monochrome=TRUE;
            textcolor(7);
            Clear_Screen();
            Print_Centered(1,"Fixed Disk Drive Status",0);
            Display_All_Drives();
            exit(0);
        }
        if(0==stricmp("/X",argv[1])) {
            if( (flags.version==W95B) || (flags.version==W98) ) Ask_User_About_FAT32_Support();
            flags.extended_int_13=FALSE;
            Interactive_User_Interface();
            exit(0);
        }
    }
    if(0==stricmp("/?",argv[1])) {
        Display_Help_Screen();
        exit(0);
    }
    if(argc>=3) flags.drive_number=((argv[2] [0])-48)+127;
    else flags.drive_number=0x80;
    if( (flags.drive_number<0x80) || (flags.drive_number>flags.maximum_drive_number) ) {
        printf("\nIncorrect drive designation...Operation Terminated.\n");
        exit(5);
    }
    if(argc>=2) {
        if(0==stricmp("/A",argv[1])) {
            Automatically_Partition_Hard_Drive();
            Write_Partition_Tables();
            exit(0);
        }
        if(0==stricmp("/ACTIVATE",argv[1])) {
            if(argc<4) {
                printf("\nSyntax Error...Operation Terminated.\n");
                exit(1);
            }
            int partition_number=atoi(argv[3]);
            if((partition_number<1) || (partition_number>4)) {
                printf("\nPartition number is out of range (1-4)...Operation Terminated.\n");
                exit(9);
            }
            Set_Active_Partition(partition_number-1);
            Write_Partition_Tables();
            exit(0);
        }
        if(0==stricmp("/AMBR",argv[1])) {
            Create_Alternate_MBR();
            exit(0);
        }
        if(0==stricmp("/C",argv[1])) {
            Clear_Partition_Table();
            exit(0);
        }
        if(0==stricmp("/CLEARFLAG",argv[1])) {
            printf("\nThe function selected has not yet been implemented...Operation Terminated.\n");
            exit(10);
            exit(0);
        }
        if(0==stricmp("/D",argv[1])) {
            int partition_number;
            if(argc<4) {
                printf("\nSyntax Error...Operation Terminated.\n");
                exit(1);
            }
            if(0==stricmp("/L",argv[3])) {
                if(partition_table[(flags.drive_number-128)].number_of_logical_drives==0) {
                    printf("\nNo Logical DOS Drives exist...Operation Terminated.\n");
                    exit(9);
                }
                printf("\nThe function selected has not yet been implemented...Operation Terminated.\n");
                exit(10);
            }
            if(0==stricmp("/E",argv[3])) {
                if(partition_table[(flags.drive_number-128)].extended_partition_exists!=TRUE) {
                    printf("\nExtended partitition does not exist...Operation Terminated.\n");
                    exit(9);
                }
                Delete_Primary_Partition(partition_table[(flags.drive_number-128)].number_of_extended_partition);
                Write_Partition_Tables();
                exit(0);
            }
            partition_number=atoi(argv[3]);
            if((partition_number<1) || (partition_number>4)) {
                printf("\nPartition number is out of range (1-4)...Operation Terminated.\n");
                exit(9);
            }
            Delete_Primary_Partition(partition_number);
            Write_Partition_Tables();
            exit(0);
        }
        if(0==stricmp("/DEACTIVATE",argv[1])) {
            Clear_Active_Partition();
            Write_Partition_Tables();
            exit(0);
        }
        if(0==stricmp("/DUMP",argv[1])) {
            Dump_Partition_Information();
            exit(0);
        }
        if(0==stricmp("/I",argv[1])) {
            if( (argc==4) && (0==stricmp("/TECH",argv[3])) ) flags.extended_options_flag=TRUE;
            if( (argc==4) && (0!=stricmp("/TECH",argv[3])) ) {
                printf("\nSyntax Error...Operation Terminated.\n");
                exit(1);
            }
            Display_CL_Partition_Table();
            exit(0);
        }
        if(0==stricmp("/L",argv[1])) {
            flags.monochrome=TRUE;
            textcolor(7);
            Clear_Screen();
            Display_All_Drives();
            exit(0);
        }
        if(0==stricmp("/M",argv[1])) {
            int partition_number;
            int type_number;
            partition_number=atoi(argv[3]);
            if((partition_number<1) || (partition_number>4)) {
                printf("\nPrimary partition number is out of range...Operation Terminated.\n");
                exit(9);
            }
            type_number=atoi(argv[4]);
            if((type_number<=0) || (type_number>255)) {
                printf("\nNew partition type is out of range...Operation Terminated.\n");
                exit(9);
            }
            Modify_Partition_Type((partition_number-1),type_number);
            Write_Partition_Tables();
            exit(0);
        }
        if( (0==stricmp("/MBR",argv[1])) || (0==stricmp("/CMBR",argv[1])) ) {
            Create_MBR();
            exit(0);
        }
        if(0==stricmp("/N",argv[1])) {
            int numeric_type;
            int partition_type=NULL;
            int special_flag=FALSE;
            unsigned long maximum_partition_size_in_MB;
            unsigned long size;
            if(argc<5) {
                printf("\nSyntax Error...Operation Terminated.\n");
                exit(1);
            }
            Determine_Free_Space();
            size=atol(argv[4]);
            if(0==stricmp("P",argv[3])) partition_type=PRIMARY;
            if(0==stricmp("E",argv[3])) partition_type=EXTENDED;
            if(0==stricmp("L",argv[3])) {
                if(partition_table[(flags.drive_number-128)].extended_partition_exists==FALSE) {
                    printf("\nNo Extended Partition Exists...Operation Terminated.\n");
                    exit(9);
                }
                partition_type=LOGICAL;
                if(1==Determine_Drive_Letters()) {
                    printf("\nMaximum number of Logical DOS Drives installed...Operation Terminated.\n");
                    exit(9);
                }
            }
            if(partition_type==NULL) {
                printf("\nSyntax Error...Operation Terminated.\n");
                exit(1);
            }
            if( ( (argc==6) || (argc==8) ) && ( (0==stricmp("/P",argv[5])) || (0==stricmp("/P",argv[7])) ) ) {
                Determine_Free_Space();
                if( (partition_type==PRIMARY) || (partition_type==EXTENDED) )size=(size*(((partition_table[(flags.drive_number-128)].primary_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048))/100;
                if(partition_type==LOGICAL) size=(size*(((partition_table[(flags.drive_number-128)].extended_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048))/100;
            }
            if( (argc>=6) && (argc<=8) && (0==stricmp("/S",argv[(argc-2)]) ) ) {
                if(0==stricmp("/S",argv[5])) numeric_type=atoi(argv[6]);
                if( (argc==8) && (0==stricmp("/S",argv[6])) ) numeric_type=atoi(argv[7]);
                if( (numeric_type<1) || (numeric_type>255) ) {
                    printf("\nSpecial partition type is out of range...Operation Terminated.\n");
                    exit(9);
                }
                special_flag=TRUE;
            }
            if( (partition_type==PRIMARY) || (partition_type==EXTENDED) ) {
                maximum_partition_size_in_MB=(((partition_table[(flags.drive_number-128)].primary_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048);
                if( (flags.version==FOUR) && (partition_type!=EXTENDED) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( (flags.version==FIVE) && (partition_type!=EXTENDED) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( (flags.version==SIX) && (partition_type!=EXTENDED) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( (flags.version==W95) && (partition_type!=EXTENDED) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==FALSE) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if(size>maximum_partition_size_in_MB) size=maximum_partition_size_in_MB;
                if(special_flag==FALSE) {
                    if(partition_type!=EXTENDED) {
                        if(size<=16) numeric_type=1;
                        if( (size>16) && (size<=32) ) numeric_type=4;
                        if(size>32) numeric_type=6;
                        if( (size>512) && ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==TRUE) ) numeric_type=0x0b;
                    } else numeric_type=5;
                }
                Create_Primary_Partition(numeric_type,size);
            } else {
                maximum_partition_size_in_MB=(((partition_table[(flags.drive_number-128)].extended_partition_largest_free_space+1)*(partition_table[(flags.drive_number-128)].total_heads+1)*(partition_table[(flags.drive_number-128)].total_sectors))/2048);
                if( (flags.version==FOUR) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( (flags.version==FIVE) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( (flags.version==SIX) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( (flags.version==W95) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if( ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==FALSE) && (maximum_partition_size_in_MB>2048) ) maximum_partition_size_in_MB=2048;
                if(size>maximum_partition_size_in_MB) size=maximum_partition_size_in_MB;
                if(special_flag==FALSE) {
                    if(size<=16) numeric_type=1;
                    if( (size>16) && (size<=32) ) numeric_type=4;
                    if(size>32) numeric_type=6;
                    if( (size>512) && ( (flags.version==W95B) || (flags.version==W98) ) && (flags.fat32==TRUE) ) numeric_type=0x0b;
                }
                Create_Logical_Drive(numeric_type,size);
            }
            Write_Partition_Tables();
            exit(0);
        }
        if(0==stricmp("/REBOOT",argv[1])) Reboot_PC();
        if(0==stricmp("/RESIZE",argv[1])) {
            printf("\nThe function selected has not yet been implemented...Operation Terminated.\n");
            exit(10);
        }
        if(0==stricmp("/RMBR",argv[1])) {
            Remove_MBR();
            exit(0);
        }
        if(0==stricmp("/SETFLAG",argv[1])) {
            printf("\nThe function selected has not yet been implemented...Operation Terminated.\n");
            exit(10);
        }
        if(0==stricmp("/SMBR",argv[1])) {
            Save_MBR();
            exit(0);
        }
        if(0==stricmp("/TESTFLAG",argv[1])) {
            printf("\nThe function selected has not yet been implemented...Operation Terminated.\n");
            exit(10);
        }
    }
    printf("\nSyntax Error...Operation Terminated.\n");
    exit(1);
}
