#include <random>
#include <iostream>
#include <tuple>
//layout: aces, twos, threes, fours, fives, sixes, thee_of_a_kind, four_of_a_kind,yahtzee, yahtzee_bonus, full_house, small_straight, large_straight, chance
struct player_state{
   uint16_t score;
   uint16_t used;
};
struct game_state{
    player_state states[2];
    uint32_t state_flags;
};
struct expanded_move{
    uint8_t move;
    uint8_t index; // number of dice that is currently selected
    uint8_t current_player;
    uint8_t dice[5];
};
std::default_random_engine generator;
std::uniform_int_distribution<uint8_t> distribution(1,6);

expanded_move expand_move(uint32_t state){
    expanded_move move;
    move.move = state&0x07;
    move.index = (state >> 3)&0x07;
    move.current_player  = (state >> 6)&0x01;
    uint32_t dice_flags  = (state >> 7);
    std::cout << (uint32_t)dice_flags << "\n";
    for(auto i = 0;i<5;i++){
        move.dice[i] = dice_flags&0x07;
        dice_flags = dice_flags >> 3;

    }
    return move;
}
uint32_t encode_move(expanded_move move){
    uint32_t res;
    for(auto i = 4; i>=0; i--){
        res = res << 3;
        res|= move.dice[i]&0x07;

    }
    res = res << 1;
    res |= move.current_player&0x01;
    res = res << 3;
    res |= move.index&0x07;
    res = res << 3;
    res |= move.move&0x07;
    return res&0x1FFFFFF;

}

int main(){
    uint32_t num = 0xFFFFFFFF;
    printf("0x%X, 0x%X\n", encode_move(expand_move(num)), encode_move(expand_move(encode_move(expand_move(num)))));
}
uint8_t max_num(uint8_t * dice){
    uint8_t max = 0;
    for(uint8_t i = 0;i<5;i++){
        uint8_t temp = 0;
        for(uint8_t j = 0;j<5;j++){
            if(dice[i]==dice[j]) temp++;
        }
        max = temp>max?temp:max;
    }
    return max;
}
uint8_t sum_dice(uint8_t * dice){
    uint8_t sum = 0;
    for(uint8_t i = 0;i<5;i++){
        sum+=dice[i];
    }
    return sum;
}
std::tuple<bool,game_state> make_choice(game_state state, uint8_t choice){
    auto move = expand_move(state.state_flags);
    if((state.states[move.current_player].used&(1<<choice))>0){
        return std::tuple<bool, game_state> {false, state};
    }
    if(choice < 6){
        auto dice_sum = 0;
        for(auto i = 0;i<5; i++ ){
            dice_sum += move.dice[i]==choice+1?choice+1:0;
        }
        if(dice_sum <= choice+1 ){
            dice_sum = 0;
        }
        state.states[move.current_player].score+=dice_sum;
        state.states[move.current_player].used|=(1<<choice);

    }
    uint8_t max = max_num(move.dice);
    uint8_t sum = sum_dice(move.dice);
    switch(choice){
        case 8:
            if(max<choice-3 ){
                state.states[move.current_player].used|=(1<<9);
            }
        case 6:
        case 7:

            if(max<choice-3 ){
                sum = 0;
            }
            state.states[move.current_player].score+=sum;
            state.states[move.current_player].used|=(1<<choice);

            break;

    }
    move.index = 0;
    move.current_player = 1-move.current_player;
    move.move = 0;
    state.state_flags = encode_move(move);

}

std::tuple<bool, game_state> next_turn(game_state state, uint8_t turn){ // returns next turn -- turn is a bitfield if turn number is 0 or 1, scoring if turn 2
    auto move = expand_move(state.state_flags);
    bool selected[5];
    if(move.move == 2){
        return make_choice(state, turn);

    }
    for(auto i = move.index; i<5;i++){
        bool flag = turn&0x01;
        turn = turn >> 0x01;
        if(flag){
            auto temp = move.dice[i];
            move.dice[i] = move.dice[move.index];
            move.dice[move.index] = temp;
            move.index++;
        }
    }
    for(auto i = move.index; i<5;i++){
        move.dice[i] = distribution(generator);
    }
    move.move++;
    state.state_flags = encode_move(move);
    return std::tuple<bool, game_state> (true, state);



}
enum win_types{
  three_of_a_kind, four_of_a_kind,yahtzee, yahtzee_bonus, full_house, small_straight, large_straight, chance

  };

}
uint8_t get_win_types(expanded_move last_move, win_type t){
  std::vector<int> dice;
  for(auto i = 0;i<last_move.index;i++){
    dice.emplace_back(last_move.dice[i]);
  }
  int instances[6];
  int sum = 0;
  for (auto i=0; i<dice.size(); i++){
    instances[dice[i]-1]++;
    sum += dice[i];
  }

  std::vector<int> zeros;
  for (auto i=0; i<6; i++){
    if (instances[i] == 0){
      zeros.push_back(i);
    }
  }
  int maxDist = 0;
  for (auto i=0; i<zeros.size(); i++){
    for (auto j=i+1; j<zeros.size(); j++){
      int dist = abs(zeros[i] - zeros[j]);
      maxDist = maxDist>dist?maxDist:dist;
    }
  }

  switch(t){
    case three_of_a_kind:
      for (auto i=6; i>=1; i--){
        if (instances[i-1] >= 3){
          return instances[i-1]*3;
        }
      }
      return 0;
    case four_of_a_kind:
      for (auto i=6; i>=1; i--){
        if (instances[i-1] >= 4){
          return sum;
        }
      }
      return 0;
    case yahtzee:
      for (auto i=6; i>=1; i--){
        if (instances[i-1] >= 5){
          return 50;
        }
      }
      return 0;
    case full_house:
      (for auto i=0; i<6; i++){
        for (auto j=0; j<6; j++){
          if (instances[i] == 3 && instances[j] == 2){
            return 25;
          }
        }
      }
      return 0;

    case small_straight:
      //return maxDist>=4?30:0;
      if (maxDist >= 4){
        return 30;
      } else {
        return 0;
      }
    case large_straight:
      if (maxDist >= 5){
        return 40;
      } else {
        return 0;
      }
    case chance:
      return sum;
  }
}
