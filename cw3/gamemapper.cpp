#include "gamemapper.h"
#include "individual.h"

GameMapper::GameMapper()
{
}

Messages::GameResponse GameMapper::Map(const Messages::GameRequest& req) {
  Messages::GameResponse resp;

  switch (req.player().algorithm()) {
  case Messages::Player_Algorithm_LINEAR:
    Map2<Individual<RatingAlgorithm_Linear> >(req, &resp);
    break;

  case Messages::Player_Algorithm_EXPONENTIAL:
    Map2<Individual<RatingAlgorithm_Exponential> >(req, &resp);
    break;

  case Messages::Player_Algorithm_EXPONENTIAL_WITH_DISPLACEMENT:
    Map2<Individual<RatingAlgorithm_ExponentialWithDisplacement> >(req, &resp);
    break;

  default:
    std::cerr << "Bad algorithm " << req.player().algorithm() << std::endl;
  }

  return resp;
}
