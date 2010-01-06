#ifndef GAMEMAPPER_H
#define GAMEMAPPER_H

#include "messages.pb.h"

#include "blockselector_random.h"
#include "blockselector_sequence.h"
#include "tetrisboard.h"
#include "game.h"

class GameMapper {
 public:
  GameMapper();

  static Messages::GameResponse Map(const Messages::GameRequest& req);

 private:
  template <typename PlayerType>
  static void Map2(const Messages::GameRequest& req, Messages::GameResponse* res);

  template <typename PlayerType, typename SelectorType>
  static void Map3(const Messages::GameRequest& req, Messages::GameResponse* res);

  template <typename PlayerType, typename SelectorType, typename BoardType>
  static void Map4(const Messages::GameRequest& req, Messages::GameResponse* res);
};

template <typename PlayerType>
void GameMapper::Map2(const Messages::GameRequest& req, Messages::GameResponse* resp) {
  if (req.has_selector_random())
    Map3<PlayerType, BlockSelector::Random>(req, resp);
  else if (req.has_selector_sequence())
    Map3<PlayerType, BlockSelector::Sequence<> >(req, resp);
  else
    std::cerr << "Bad selector type" << std::endl;
}

template <typename PlayerType, typename SelectorType>
void GameMapper::Map3(const Messages::GameRequest& req, Messages::GameResponse* resp) {
  switch (req.board().width()) {
    case 5: Map4<PlayerType, SelectorType, TetrisBoard<5, 10> >(req, resp); break;
    case 6: Map4<PlayerType, SelectorType, TetrisBoard<6, 12> >(req, resp); break;
    case 7: Map4<PlayerType, SelectorType, TetrisBoard<7, 14> >(req, resp); break;
    case 8: Map4<PlayerType, SelectorType, TetrisBoard<8, 16> >(req, resp); break;
    case 9: Map4<PlayerType, SelectorType, TetrisBoard<9, 18> >(req, resp); break;
    case 10: Map4<PlayerType, SelectorType, TetrisBoard<10, 20> >(req, resp); break;

    default: std::cerr << "Bad board width " << req.board().width() << std::endl;
  }
}

template <typename PlayerType, typename SelectorType, typename BoardType>
void GameMapper::Map4(const Messages::GameRequest& req, Messages::GameResponse* resp) {
  PlayerType player;
  player.FromMessage(req.player());

  SelectorType selector;
  selector.FromMessage(req);

  Game<PlayerType, SelectorType, BoardType> game(player, selector);
  game.Play();

  resp->set_player_id(req.player_id());
  resp->set_selector_id(req.selector_id());
  resp->set_blocks_placed(game.BlocksPlaced());
}

#endif // GAMEMAPPER_H
