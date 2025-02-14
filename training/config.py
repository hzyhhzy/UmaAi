Game_Input_C = 1113
Game_Input_C_Global = 573
Game_Input_C_Card = 90
Game_Input_C_Person= 0
Game_Card_Num = 6
Game_Head_Num = 0
assert(Game_Input_C==Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card+Game_Head_Num*Game_Input_C_Person)

Game_Output_C = 24
Game_Output_C_Policy = 21
Game_Output_C_Value = 3
assert(Game_Output_C==Game_Output_C_Policy+Game_Output_C_Value)

Value_Mean=43000 #mean score of game
Value_Scale=300
Valuevar_Scale=150
Vloss_Scale=0.1