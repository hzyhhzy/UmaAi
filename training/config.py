Game_Input_C = 1121
Game_Input_C_Global = 587
Game_Input_C_Card = 89
Game_Input_C_Person= 0
Game_Card_Num = 6
Game_Head_Num = 0
assert(Game_Input_C==Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card+Game_Head_Num*Game_Input_C_Person)

Game_Output_C = 56
Game_Output_C_Policy = 53
Game_Output_C_Value = 3
assert(Game_Output_C==Game_Output_C_Policy+Game_Output_C_Value)