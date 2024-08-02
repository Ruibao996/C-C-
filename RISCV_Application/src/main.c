#include "lcd/lcd.h"
#include "utils.h"
#include "assembly/example.h"
#include <stdlib.h>

#define GRAVITY 1
#define JUMP_SPEED 5
#define MAX_FALL_SPEED 10
#define PLAYER_LIVES 10
#define WALL_SPEED 1
#define WALL_GAP 45
#define SCORE_X 5
#define SCORE_Y 5
#define LIVES_X 5
#define LIVES_Y 25
#define SCORE_COLOR GREEN
#define LIVES_COLOR RED
#define DEBOUNCE_DELAY 300
#define TRACE_LENGTH 15

typedef struct
{
  int x, y;
} Point;

Point trace[TRACE_LENGTH];
Point trace_ori[TRACE_LENGTH];
int trace_index = 0;
int trace_index_ori = 0;

typedef enum
{
  EASY,
  HARD
} Difficulty;

Difficulty difficulty;

typedef struct
{
  int x, y;
  int speed;
  int lives;
  int score;
} Player;

typedef struct
{
  int x, top, bottom;
} Wall;

Player player;
Player player_ori;
Wall walls[2];
Wall walls_ori[2];

void Inp_init(void)
{
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOC);

  gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
  gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

void IO_init(void)
{
  Inp_init(); // inport init
  Lcd_Init(); // LCD init
}

void delay_until_next_frame(uint64_t start_time, uint64_t frame_time)
{
  uint64_t current_time = get_timer_value();
  uint64_t elapsed_time = current_time - start_time;

  if (elapsed_time < frame_time)
  {
    uint32_t delay_time = (frame_time - elapsed_time) * 4000.0 / SystemCoreClock;
    delay_1ms(delay_time);
  }
}

void welcom_screen(void) {
  draw();
  // delay_1ms(1000);
  LCD_DrawPoint_big(15, 40, GREEN);
  delay_1ms(1000);
  LCD_DrawPoint_big(25, 40, BLUE);
  delay_1ms(1000);
  LCD_DrawPoint_big(35, 40, YELLOW);
  delay_1ms(1000);
  LCD_DrawPoint_big(45, 40, RED);
  delay_1ms(1000);
  LCD_DrawPoint_big(55, 40, GREEN);
  delay_1ms(1000);
  LCD_Clear(BLACK);
}

void difficulty_selection(void) {
  Difficulty selected_difficulty = EASY;
  uint64_t frame_time = SystemCoreClock / 50;
  Difficulty origin_difficulty = selected_difficulty;
  int down_guarantee = 15;
  int up_guarantee = 15;

  while (1) {
    uint64_t start_time = get_timer_value();
    origin_difficulty = selected_difficulty;
    // Draw difficulty selection screen
    LCD_ShowString(10, 20, (u8*)(selected_difficulty == EASY ? "> Easy" : "Easy"), WHITE);
    LCD_ShowString(10, 60, (u8*)(selected_difficulty == HARD ? "> Hard" : "Hard"), WHITE);

    // Handle input
    if (up_guarantee == 15)
    {
      if (Get_Button(JOY_UP) || Get_Button(JOY_LEFT))
      {
        if (selected_difficulty > EASY)
        {
          selected_difficulty = EASY;
        }
        up_guarantee = 0;
      }
    }

    if (down_guarantee == 15)
    {
      if (Get_Button(JOY_DOWN) || Get_Button(JOY_RIGHT))
      {
        if (selected_difficulty < HARD)
        {
          selected_difficulty = HARD;
        }
        down_guarantee = 0;
      }
    }
    
    if (Get_Button(JOY_CTR)) {
      difficulty = selected_difficulty;
      LCD_ShowString(10, 20, (u8*)"> Easy", BLACK);
      LCD_ShowString(10, 60, (u8*)"> Hard", BLACK);
      LCD_ShowString(10, 20, (u8*)"Easy", BLACK);
      LCD_ShowString(10, 60, (u8*)"Hard", BLACK); 
      break;
    }

    if (up_guarantee < 15)
    {
      up_guarantee++;
    }

    if (down_guarantee < 15)
    {
      down_guarantee++;
    }

    delay_until_next_frame(start_time, frame_time);

    if (origin_difficulty != selected_difficulty) {
      LCD_ShowString(10, 20, (u8*)"> Easy", BLACK);
      LCD_ShowString(10, 60, (u8*)"> Hard", BLACK);
      LCD_ShowString(10, 20, (u8*)"Easy", BLACK);
      LCD_ShowString(10, 60, (u8*)"Hard", BLACK);
    }
  }
}

void Plappy_bird_game(void)
{
  welcom_screen();

  // Difficulty selection
  difficulty_selection();

  // Initialize player
  player.x = 105;
  player.y = 10;
  player.speed = 0;
  player.lives = PLAYER_LIVES;
  player.score = 0;

  // Clear trace
  for (int i = 0; i < TRACE_LENGTH; i++)
  {
    trace[i].x = -1;
    trace[i].y = -1;
  }

  // Initialize walls
  for (int i = 0; i < 2; i++)
  {
    walls[i].x = 160 + i * WALL_GAP;
    int height = rand() % 30 + 10;
    walls[i].top = height - 10;
    walls[i].bottom = height + 20;
  }

  // Set game parameters based on difficulty
  int wall_speed;
  switch (difficulty)
  {
  case EASY:
    wall_speed = 1;
    break;
  case HARD:
    wall_speed = 3;
    break;
  }

  uint64_t frame_time = SystemCoreClock / 50;

  int trace_color = 1;
  bool player_color = 1;
  bool first_time = 1;
  int len_live = 1;
  int len_score= 1;
  int len_live_ori = 1;
  int len_score_ori = 1;
  int origin_live = 0;
  int origin_score = 0;

  while (1)
  {
    uint64_t start_time = get_timer_value();

    LCD_ShowString(LIVES_X, LIVES_Y, (u8 *)"Lives:", LIVES_COLOR);
    LCD_ShowString(SCORE_X, SCORE_Y, (u8 *)"Score:", SCORE_COLOR);
    LCD_DrawRectangle(70, 1, 159, 79, YELLOW);

    origin_live = player.lives;
    origin_score = player.score;
    len_live_ori = len_live;
    len_score_ori = len_score;


    // Handle player input
    if (Get_Button(JOY_UP))
    {
      player.speed = -JUMP_SPEED;
      
      for (int i = 0; i < abs(player.speed); i++)
      {
        trace[trace_index].x = player.x;
        trace[trace_index].y = player.y;
        trace_index = (trace_index + 1) % TRACE_LENGTH;
        trace_color = 0;
      }
    }
    else
    {
      player.speed += GRAVITY;
      if (player.speed > MAX_FALL_SPEED)
      {
        player.speed = MAX_FALL_SPEED;
      }
    }

    // Move player
    player.y += player.speed;

    // Add player position to trace
    trace[trace_index].x = player.x;
    trace[trace_index].y = player.y;
    trace_index = (trace_index + 1) % TRACE_LENGTH;

    // Update trace x coordinates
    for (int i = 0; i < TRACE_LENGTH; i++)
    {
      if (trace[i].x != -1)
      {
        trace[i].x -= 1;
      }
    }

    // Check collision with walls
    for (int i = 0; i < 2; i++)
    {
      if ((((player.x >= walls[i].x) && (player.x <= walls[i].x + 1)) && (player.y < walls[i].top || player.y > walls[i].bottom)) || player.y < 0 || player.y > 80)
      {
        player.lives--;
        player.x = 105;
        player.y = walls[i].bottom - 25;
        player_color = !player_color;
        for (int i = 0; i < TRACE_LENGTH; i++)
        {
          trace[i].x = -1;
          trace[i].y = -1;
        }
      }

      // Increase score when player passes a wall
      if (player.x == walls[i].x && player.y >= walls[i].top && player.y <= walls[i].bottom)
      {
        player.score++;
      }

      // Move walls
      walls[i].x -= wall_speed;
      if (walls[i].x < 71)
      {
        walls[i].x = 159;
        int height = rand() % 30 + 20;
        walls[i].top = height - 10;
        walls[i].bottom = height + 20;
      }

      // Reset lives and score if lives are 0 or player is out of screen
      if (player.lives == 0)
      {
        player.lives = PLAYER_LIVES;
        player.score = 0;
        player.x = 105;
        player.y = 10;
        player_color = !player_color;
      }
    }

    if (player_ori.lives < 10)
    {
      len_live_ori = 1;
    }
    else
    {
      len_live_ori = 2;
    }

    if (player_ori.score < 10)
    {
      len_score_ori = 1;
    }
    else
    {
      len_score_ori = 2;
    }

    if (player.lives < 10)
    {
      len_live = 1;
    }
    else
    {
      len_live = 2;
    }

    if (player.score < 10)
    {
      len_score = 1;
    }
    else
    {
      len_score = 2;
    }

    // Clear origin lives
    if (!first_time)
    {
      LCD_ShowNum(LIVES_X + 48, LIVES_Y,player_ori.lives, len_live_ori, BLACK);
    }

    // Draw player lives
    LCD_ShowNum(LIVES_X + 48, LIVES_Y,player.lives, len_live, LIVES_COLOR);

    // Clear origin score
    if (!first_time)
    {
      LCD_ShowNum(SCORE_X + 48, SCORE_Y, player_ori.score, len_score_ori, BLACK);
    }

    // Draw player score
    LCD_ShowNum(SCORE_X + 48, SCORE_Y, player.score, len_score, SCORE_COLOR);

    // Clear origin trace
    if (!first_time)
    {
      for (int i = 0; i < TRACE_LENGTH - 1; i++)
      {
        if (trace_ori[i].x > 70)
        {
          LCD_DrawLine(trace_ori[i].x, trace_ori[i].y, trace_ori[i + 1].x, trace_ori[i + 1].y, BLACK);
        }
      }
      LCD_DrawLine(trace_ori[TRACE_LENGTH - 1].x, trace_ori[TRACE_LENGTH - 1].y, trace_ori[0].x, trace_ori[0].y, BLACK);
    }

    // Draw trace
    for (int i = trace_index; i < TRACE_LENGTH - 1; i++)
    {
      if (trace[i].x > 70 && trace[i+1].x > 70)
      {
      if (trace_color) {
        if (player_color)
          LCD_DrawLine(trace[i].x, trace[i].y, trace[i+1].x, trace[i+1].y, BLUE);
        else
          LCD_DrawLine(trace[i].x, trace[i].y, trace[i+1].x, trace[i+1].y, GREEN);
      }
        else {
          LCD_DrawLine(trace[i].x, trace[i].y, trace[i + 1].x, trace[i + 1].y, RED);
          trace_color = 1;
        }
      }
    }
    for (int i = 0; i < trace_index - 1; i++)
    {
      if (trace[i].x > 70 && trace[i+1].x > 70)
      {
      if (trace_color) {
        if (player_color)
          LCD_DrawLine(trace[i].x, trace[i].y, trace[i+1].x, trace[i+1].y, BLUE);
        else
          LCD_DrawLine(trace[i].x, trace[i].y, trace[i+1].x, trace[i+1].y, GREEN);
      }
        else {
          LCD_DrawLine(trace[i].x, trace[i].y, trace[i + 1].x, trace[i + 1].y, RED);
          trace_color = 1;
        }
      }
    }

    if (trace[TRACE_LENGTH - 1].x > 70 && trace[0].x > 70 && abs(trace[TRACE_LENGTH - 1].x - trace[0].x) < 4)
    {
      if (trace_color) {
        if (player_color)
          LCD_DrawLine(trace[TRACE_LENGTH - 1].x, trace[TRACE_LENGTH - 1].y, trace[0].x, trace[0].y, BLUE);
        else
          LCD_DrawLine(trace[TRACE_LENGTH - 1].x, trace[TRACE_LENGTH - 1].y, trace[0].x, trace[0].y, GREEN);
      }
      else {
        LCD_DrawLine(trace[TRACE_LENGTH - 1].x, trace[TRACE_LENGTH - 1].y, trace[0].x, trace[0].y, RED);
        trace_color = 1;
      }
    }

    // Clear origin player
    if (!first_time)
    {
      LCD_DrawPoint_big(player_ori.x, player_ori.y, BLACK);
    }

    // Draw player
    if (player_color)
      LCD_DrawPoint_big(player.x, player.y, GREEN);
    else {
      LCD_DrawPoint_big(player.x, player.y, YELLOW);
    }

    // Clear origin walls
    if (!first_time)
    {
      for (int i = 0; i < 2; i++)
      {
        LCD_DrawRectangle(walls_ori[i].x, 2, walls_ori[i].x + 1, walls_ori[i].top, BLACK);
        LCD_DrawRectangle(walls_ori[i].x, walls_ori[i].bottom, walls_ori[i].x + 1, 78, BLACK);
      }
    }

    first_time = 0;
    // Draw walls
    for (int i = 0; i < 2; i++)
    {
      LCD_DrawRectangle(walls[i].x, 2, walls[i].x + 1, walls[i].top, RED);
      LCD_DrawRectangle(walls[i].x, walls[i].bottom, walls[i].x + 1, 78, WHITE);
    }


    player_ori = player;
    walls_ori[0] = walls[0];
    walls_ori[1] = walls[1];
    trace_index_ori = trace_index;
    for (int i = 0; i < TRACE_LENGTH; i++)
    {
      trace_ori[i] = trace[i];
    }

    delay_until_next_frame(start_time, frame_time);
  }
}

int main(void)
{
  IO_init();
  LCD_Clear(BLACK);
  Plappy_bird_game();
}
