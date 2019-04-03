#include <stdlib.h>
#include <stdbool.h>
typedef struct b{       //3d box struct
    int box_id;         //what is the box number from the top-down left to right(0-27)
    int rowNum;         //what row on the playing field it is
    int colNum;         //what col on the playing field it is
    bool _is_highlighted; //initial value is false
}box;
    
typedef struct f{   //actual playing field struct , contains all boxes
    int num_highlighted; //number of blocks on the field that are highlighted 
    box list_of_boxes[28];  //array 
    
}field;

typedef struct{
    //information about what block the player is standing on
    int curr_id;
    int curr_row;
    int curr_col;
    int player_type; //1 for main player , 2+ for enemy types
    int lives; //number of lives the player has left

}player;

volatile int pixel_buffer_start; // global variable
volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address
    volatile int * interval_timer_ptr = (int *) 0xFF202000; // Altera timer address

void draw_line(int x0, int x1, int y0, int  y1, int rgb);
void plot_pixel(int x, int y, short int line_color);
void plot_square(int x, int y, int line_color);
void plot_3_square(int x, int y, int line_color);
void clear_screen();
void wait_for_vsync();
bool checkWin(field* game_field);
void setHighlighted(int id, field* game_field);
void setunHighlighted(int id, field* game_field);
void draw_field(field* game_field);
void draw_player(player*player);
void initBoard(field* game_field);
void initPlayer(player* player);
void initPlayer2(player* player, int r, int c, int type);
bool getHighlighted(int id, field* game_field);
int rowCol_to_id(int row, int col);
void movePlayer(player * player, int key_value, int player_type, field* board);
void setPlayerPosition(player* player, int r, int c);
void simpleEnemyMove(player* enemy, field* board);
void draw_cube(int x_center, int y_center_top, int topColor, int leftColor, int rightColor);
void checkValidMove(player* player, int key_value, field* board);
//interrupt stuff

void set_A9_IRQ_stack (void);
void config_GIC (void);
void config_interval_timer (void);
void enable_A9_interrupts (void);

int count = 1;

field game_field;
field* board = &game_field;
player p1;
player* p1_ptr = &p1;
player e1; //enemy
player* e1_ptr = &e1;


void initBoard(field* game_field){
    game_field->num_highlighted =0;
    //initialize box values
    for(int r =0; r<7; r++){
        for(int c = 0; c<r+1;c++){
            int id = rowCol_to_id(r, c);
            game_field->list_of_boxes[id].rowNum = r;
            game_field->list_of_boxes[id].colNum = c;
            game_field->list_of_boxes[id].box_id = id;
            game_field->list_of_boxes[id]._is_highlighted = false;
        }
    }
}

void initPlayer(player* player){
    player->lives = 3;
    player->player_type = 1;
    player->curr_row= 0;
    player->curr_col =0;
    player->curr_id = 0;
    
}

void initPlayer2(player* player, int r, int c, int type){
    player->lives = 3;
    player->player_type = type;
    player->curr_row= r;
    player->curr_col =c;
    player->curr_id = rowCol_to_id(r, c);
    
}
void draw_player(player*player){
    //player is a 3x3 red square
    int r = player->curr_row;
    int c = player->curr_col;
        int left_margin = (320 -(26*(r+1)))/2;
    int x = left_margin + (26*(c)) - 2;
    int y = 10 + (26*r) + 6; 
    plot_3_square(x, y, 0xfac4);
}

void draw_field(field* game_field){   
    for(int r =0; r<7; r++){
        int left_margin = (320 -(26*(r+1)))/2;
    	for(int c =0;c<r+1;c++){
        	int y = 10 + (26*r); 
            int x = left_margin + (26*(c));
            int id = rowCol_to_id(r, c);
            if(game_field->list_of_boxes[id]._is_highlighted){
            	draw_cube(x, y, 0x1111, 0xffff, 0xf);
            }
            else{
            	draw_cube(x, y, 0xfff, 0xffff, 0xf);
            }
        }
    }
}

void movePlayer(player * player, int key_value, int player_type, field* board){
            //get current player values   
            int r = player->curr_row;
            int c = player->curr_col;
            int id = rowCol_to_id(r, c);
    
            if(key_value == 1){//down key--> down, left  (row increase)
                if(rowCol_to_id(r+1, c) != -1){//if valid move
                    setPlayerPosition(player, r+1, c);
                    id = rowCol_to_id(r+1, c);
                    if(player_type == 1){
                        setHighlighted(id, board);
                    }
                    else if (player_type == 2){
                        setunHighlighted(id, board);
                    }       
                }       
            }
            
            if(key_value == 2){//up key --> up, right (row decrease)
                 if(rowCol_to_id(r-1, c) != -1){//if valid move
                    setPlayerPosition(player, r-1, c);
                    id = rowCol_to_id(r-1, c);
                    if(player_type == 1){
                        setHighlighted(id, board);
                    }
                    else if (player_type == 2){
                        setunHighlighted(id, board);
                    }
                }       
            
            }
            
            if(key_value == 4){//left key--> up, left (row decrease, col decrease)
                if(rowCol_to_id(r-1, c-1) != -1){//if valid move
                    setPlayerPosition(player, r-1, c-1);
                    id = rowCol_to_id(r-1, c-1);
                    if(player_type == 1){
                        setHighlighted(id, board);
                    }
                    else if (player_type == 2){
                        setunHighlighted(id, board);
                    }
                }       
            
            }
            
            if(key_value == 8){//right key --> down, right (row increase, col increase)
                if(rowCol_to_id(r+1, c+1) != -1){//if valid move
                    setPlayerPosition(player, r+1, c+1);
                    id = rowCol_to_id(r+1, c+1);
                    if(player_type == 1){
                        setHighlighted(id, board);
                    }
                    else if (player_type == 2){
                        setunHighlighted(id, board);
                    }
                }       
            }

}

void simpleEnemyMove(player* enemy, field* board){
    if(enemy->curr_row == 6){
    	int row_start = 0;
    	int col_start = rand()%(row_start+1);
        int enemyType = (rand()%2)+2;
    	initPlayer2(e1_ptr,row_start, col_start, enemyType);
    }
    //select random value and move to an adjacent spot
    int move_type = rand()%2;
    if(move_type == 0)
    	movePlayer(enemy, 1, enemy->player_type, board);
    else
       movePlayer(enemy, 8, enemy->player_type, board);

}
void setPlayerPosition(player* player, int r, int c){
    player->curr_row = r;
    player->curr_col = c;
    player->curr_id = rowCol_to_id(r, c);
}
bool checkWin(field* game_field){
    if(game_field->num_highlighted == 28){
        return true;
    }
    return false;
}
    
void setHighlighted(int id, field* game_field){
    
    if(!(game_field->list_of_boxes[id]._is_highlighted)){
        game_field->list_of_boxes[id]._is_highlighted = true;
       (game_field->num_highlighted)++;
    }
}

void setunHighlighted(int id, field* game_field){
    
    if((game_field->list_of_boxes[id]._is_highlighted)){
        game_field->list_of_boxes[id]._is_highlighted = false;
       (game_field->num_highlighted)--;
    }
}

bool getHighlighted(int id, field* game_field){  
    return(game_field->list_of_boxes[id]._is_highlighted);
}
int main(void)
{
    
    //interrupt stuff
    set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
    config_GIC(); // configure the general interrupt controller
    config_interval_timer(); // configure Altera interval timer to generate interrupts     
    enable_A9_interrupts(); // enable interrupts in the A9 processor
    
    int flag = 0; //set 1 on make code, set 0 on break code
    unsigned char byte1 = 0;
    unsigned char byte2 = 0;
    unsigned char byte3 = 0;
    int PS2_data, RVALID;
    
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    
    volatile int* edge_capture_ptr = (int *)0xFF20005C;
    
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

	
    initBoard(board);
    
    initPlayer(p1_ptr);
    
    int row_start = 0;
    int col_start = rand()%(row_start+1);
    int enemyType = (rand()%2)+2;
    initPlayer2(e1_ptr,row_start, col_start, enemyType);
    
    
        clear_screen();
   
        //setHighlighted(27, board);
        draw_field(board);
        draw_player(p1_ptr);
        draw_player(e1_ptr);
    
        while(p1_ptr->lives >0){
            if(checkWin(board)){
                clear_screen();
                while(1);//idle here 
            }
        PS2_data = *(PS2_ptr);  // read the Data register in the PS/2 port
        RVALID = (PS2_data & 0x8000);   // extract the RVALID field
        if (RVALID != 0){
                
                if(flag ==0){
                    flag =1;
                }
                else{flag = 0;}
                byte1 = byte2;
                byte2 = byte3;
                byte3 = PS2_data & 0xFF;
                
                //left, right, up, down 
                if(byte3 == 0x6B && flag == 1){//left
                    movePlayer(p1_ptr,4 ,1, board);
                    
                    //check collision
                    if(p1_ptr->curr_id == e1_ptr->curr_id){
                        p1_ptr->lives--;
                        p1_ptr->curr_id =0;
                        p1_ptr->curr_row =0;
                        p1_ptr->curr_col =0;
                    }                    
                    draw_field(board);
                    draw_player(p1_ptr);
                    draw_player(e1_ptr);
                }
                if(byte3 == 0x75 && flag == 1){//up
                    movePlayer(p1_ptr, 2, 1,  board);
                                        
                    //check collision
                    if(p1_ptr->curr_id == e1_ptr->curr_id){
                        p1_ptr->lives--;
                        p1_ptr->curr_id =0;
                        p1_ptr->curr_row =0;
                        p1_ptr->curr_col =0;
                    } 
                    draw_field(board);
                    draw_player(p1_ptr);
                    draw_player(e1_ptr);
                }
                if(byte3 == 0x72 & flag == 1){//down
                    movePlayer(p1_ptr,1 , 1, board);
                                        
                    //check collision
                    if(p1_ptr->curr_id == e1_ptr->curr_id){
                        p1_ptr->lives--;
                        p1_ptr->curr_id =0;
                        p1_ptr->curr_row =0;
                        p1_ptr->curr_col =0;
                    } 
                    draw_field(board);
                    draw_player(p1_ptr);
                    draw_player(e1_ptr);
                }
                if(byte3 == 0x74 && flag == 1){//right
                    movePlayer(p1_ptr, 8, 1, board);
                                        
                    //check collision
                    if(p1_ptr->curr_id == e1_ptr->curr_id){
                        p1_ptr->lives--;
                        p1_ptr->curr_id =0;
                        p1_ptr->curr_row =0;
                        p1_ptr->curr_col =0;
                    } 
                    draw_field(board);
                    draw_player(p1_ptr);
                    draw_player(e1_ptr);
                }
            }
       
        }
    
    while(1);
}//end main

void draw_cube(int x_center, int y_center_top, int topColor, int leftColor, int rightColor){
	//top
    
    for(int y = 0; y<13; y++){
    	for(int x =0; x<y; x++){
       	 	plot_pixel(x_center+x, y_center_top+y, topColor);
            plot_pixel(x_center-x, y_center_top+y, topColor);
        }
    }
    

    
    //bottom
    
    for(int y = 13; y<26; y++){
    	for(int x = 0; x<26-y; x++){
       	 	plot_pixel(x_center+x, y_center_top+y, topColor);
            plot_pixel(x_center-x, y_center_top+y, topColor);
        }
    }
    
	    
    //left side
    for(int i =0;i <13; i++){
    	for(int j =0; j<13; j++){
        	plot_pixel((x_center-13)+j, (y_center_top+13+i)+j, leftColor);
        }
    }
    
    
    //right side
    for(int i =0;i <13; i++){
   		for(int j =0; j<13; j++){
        	plot_pixel((x_center+13)-j, (y_center_top+13+i)+j, rightColor);
        }
    }
    
}
void wait_for_vsync(){
    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
    register int status;
    
    *pixel_ctrl_ptr = 1;
    
    status = *(pixel_ctrl_ptr +3);
    while((status & 0x01) != 0){
        status = *(pixel_ctrl_ptr+3);
    }
}

void checkValidMove(player* player, int key_value, field* board){
	//assume player is still on a valid position

	return -1; //if invalid move (falls off screen)
}
int rowCol_to_id(int row, int col){
    if(col > row || row<0 || col<0){   //if col greater than row, out of bounds
        return -1;
    }
    
    switch(row){
        case 0:
            return 0;
            
        case 1:
            return 1+col;
            
        case 2:
            return 3+col;
            
        case 3:
            return 6+col;
            
        case 4:
            return 10+col;
            
        case 5:
            return 15+col;
            
        case 6:
            return 21+col;
            
        default: return -1;    
    }
}
void plot_square(int x, int y, int line_color){//assume 15x15
    for(int i =0; i<14; i++){
        for(int j =0; j<14; j++){
            plot_pixel(x+i, y+j, line_color);
        }
    }
}

void plot_3_square(int x, int y, int line_color){//assume 5x5
    for(int i =0; i<4; i++){
        for(int j =0; j<4; j++){
            plot_pixel(x+i, y+j, line_color);
        }
    }
}

// code not shown for clear_screen() and draw_line() subroutines
void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen(){
    for(int i =0; i<320; i++){
        for(int j=0; j<240; j++){
            plot_pixel(i, j, 0);
        }
    }
}
void draw_line(int x0, int y0, int x1, int y1, int rgb){
    //check if line is steep
    bool is_steep = abs(y1-y0) > abs(x1-x0);
    
    //if it is
    if(is_steep){
        //swap x0 and y0
        //swap x1 and y1
        int temp = x0;
        x0 =y0;
        y0 = temp;
        
        temp = x1;
        x1 = y1;
        y1 = temp;     
    }
    
    //if x0 > x1
    if(x0 > x1){
    //swap x0 and x1
    //swap y0 and y1
        
     int temp = x1;
     x1 = x0;
     x0 = temp;
        
    temp =y0;
    y0 = y1;
    y1 = temp;
    
    }
    
    int deltax = x1-x0;
    int deltay = abs(y1-y0);
    int error = -(deltax/2);
    int y = y0;
    int y_step;
    
    if( y0 < y1 ){
        y_step = 1;
    }
    else{
       y_step = -1;
    }
    
    
    for(int x = x0; x<x1; x++){
        if(is_steep){
            plot_pixel(y,x, rgb);
        }
        else{
            plot_pixel(x,y, rgb);
        }
        error = error+deltay;
        if(error>=0){
            y = y + y_step;
            error = error - deltax;
        }
        
    }
    
    
}


/* Initialize the banked stack pointer register for IRQ mode */
void set_A9_IRQ_stack(void)
{
    int stack, mode;
    stack = 0xFFFFFFFF - 7; // top of A9 on-chip memory, aligned to 8 bytes
    /* change processor to IRQ mode with interrupts disabled */
    mode = 0b11010010;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r" (stack));
    /* go back to SVC mode before executing subroutine return! */
    mode = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
}

/* Turn on interrupts in the ARM processor */
void enable_A9_interrupts(void)
{
    int status = 0b01010011;
    asm("msr cpsr, %[ps]" : : [ps]"r"(status));
}

/* Configure the Generic Interrupt Controller (GIC) */
void config_GIC(void)
{

/* configure the FPGA interval timer and KEYs interrupts */
    *((int *) 0xFFFED848) = 0x00000101;
    *((int *) 0xFFFED108) = 0x00000300;
// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all priorities
	*((int *) 0xFFFEC104) = 0xFFFF;
// Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
	*((int *) 0xFFFEC100) = 1; // enable = 1
// Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
	*((int *) 0xFFFED000) = 1; // enable = 1
    
}

/* setup the interval timer interrupts in the FPGA */
void config_interval_timer()
{
    volatile int * interval_timer_ptr = (int *) 0xFF202000; // interal timer base address
    /* set the interval timer period for scrolling the HEX displays */
    int counter = 125000000; // 1/(100 MHz)×(5000000) = 50 msec
    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
    *(interval_timer_ptr + 0x3) = (counter >>16) & 0xFFFF;
    /* start interval timer, enable its interrupts */
    *(interval_timer_ptr + 1) = 0x7; // STOP = 0, START = 1, CONT = 1, ITO = 1
}

void interval_timer_ISR (void);
void interval_timer_2_ISR (void);

/* Define the IRQ exception handler */
void __attribute__ ((interrupt)) __cs3_isr_irq (void)
{
    // Read the ICCIAR from the processor interface
    int int_ID = *((int *) 0xFFFEC10C);
    
    if (int_ID == 72) {// check if interrupt is from the Altera timer
        *((int *) 0xFFFEC110) = int_ID;
		interval_timer_ISR ();	
    }
	else 
        while (1) // if unexpected, then stay here
    
    // Write to the End of Interrupt Register (ICCEOIR)
    *((int *) 0xFFFEC110) = int_ID;
	return;
}

// Define the remaining exception handlers */
void __attribute__ ((interrupt)) __cs3_isr_undef (void)
{
while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_swi (void)
{
while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_pabort (void)
{
while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_dabort (void)
{
while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_fiq (void)
{
while (1);
}

void interval_timer_ISR( )
{   
    *(interval_timer_ptr) = 0; // clear the interrupt 
	if(p1_ptr->lives <= 0 || checkWin(board)){
    	return;
    }
    simpleEnemyMove(e1_ptr, board);
    draw_field(board);
    draw_player(p1_ptr);
    draw_player(e1_ptr);
    
    volatile int* led_ptr = (int*) 0xff200000;
    *(led_ptr) = count;
    count++;
    
}

void interval_timer_2_ISR(){

}
