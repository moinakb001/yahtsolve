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
struct player_state{
   uint16_t score;
   uint16_t used;
};
struct state{

};
struct action{

};

// (state, action) => q-value

std::unordered_map<state, std::unordered_map<action, double>> ht;
