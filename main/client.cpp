
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <vector>

#include "const.h"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"

enum class Status {
  NOT_CONNECTED,
  CONNECTED
};
enum class Color {
  kWhite,
  kBlack,
  kNuLL
};

enum class PieceType { King, Queen, Rook, Bishop, Knight, Pawn };

struct Piece {
  PieceType type;
  Color color;
  sf::Vector2i pos;
  sf::Sprite sprite;
};

bool isOccupied(const sf::Vector2i& pos, const std::vector<Piece>& pieces) {
  for (const auto& piece : pieces) {
    if (piece.pos == pos)
      return true;
  }
  return false;
}


void MovePiece(std::vector<Piece>& pieces, sf::Vector2i current_pos, sf::Vector2i new_pos) {
  for (auto& piece : pieces) {
    if (piece.pos == current_pos) {
        piece.pos = new_pos;
      break;
    }
  }
}

void RemovePiece(std::vector<Piece>& pieces, sf::Vector2i pos) {
  pieces.erase(
      std::remove_if(pieces.begin(), pieces.end(),
                     [pos](const Piece& p) { return p.pos == pos; }),
      pieces.end()
  );
}

std::vector<sf::Vector2i> getPawnMoves(const Piece& pawn,
                                       const std::vector<Piece>& whitePieces,
                                       const std::vector<Piece>& blackPieces) {
  std::vector<sf::Vector2i> moves;
  int direction = (pawn.color == Color::kWhite) ? -1 : 1;


  sf::Vector2i forward = {pawn.pos.x, pawn.pos.y + direction};
  if (forward.y >= 0 && forward.y < 8 &&
      !isOccupied(forward, whitePieces) && !isOccupied(forward, blackPieces)) {
    moves.push_back(forward);

    if ((pawn.color == Color::kWhite && pawn.pos.y == 6) ||
        (pawn.color == Color::kBlack && pawn.pos.y == 1)) {
      sf::Vector2i doubleForward = {pawn.pos.x, pawn.pos.y + 2 * direction};
      if (!isOccupied(doubleForward, whitePieces) && !isOccupied(doubleForward, blackPieces)) {
        moves.push_back(doubleForward);
      }
    }
  }


  sf::Vector2i diagLeft = {pawn.pos.x - 1, pawn.pos.y + direction};
  if (diagLeft.x >= 0 && diagLeft.x < 8 && diagLeft.y >= 0 && diagLeft.y < 8 &&
      isOccupied(diagLeft, (pawn.color == Color::kWhite ? blackPieces : whitePieces))) {
    moves.push_back(diagLeft);
  }


  sf::Vector2i diagRight = {pawn.pos.x + 1, pawn.pos.y + direction};
  if (diagRight.x >= 0 && diagRight.x < 8 && diagRight.y >= 0 && diagRight.y < 8 &&
      isOccupied(diagRight, (pawn.color == Color::kWhite ? blackPieces : whitePieces))) {
    moves.push_back(diagRight);
  }

  return moves;
}


std::vector<sf::Vector2i> getKnightMoves(const Piece& knight,
                                         const std::vector<Piece>& whitePieces,
                                         const std::vector<Piece>& blackPieces) {
  std::vector<sf::Vector2i> moves;

  const std::vector<Piece>& ownPieces = (knight.color == Color::kWhite ? whitePieces : blackPieces);


  int offsets[8][2] = { {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
                        {1, 2}, {1, -2}, {-1, 2}, {-1, -2} };

  for (int i = 0; i < 8; ++i) {
    sf::Vector2i newPos = { knight.pos.x + offsets[i][0], knight.pos.y + offsets[i][1] };
    if (newPos.x >= 0 && newPos.x < 8 && newPos.y >= 0 && newPos.y < 8) {

      if (!isOccupied(newPos, ownPieces))
        moves.push_back(newPos);
    }
  }
  return moves;
}


std::vector<sf::Vector2i> getBishopMoves(const Piece& bishop,
                                         const std::vector<Piece>& whitePieces,
                                         const std::vector<Piece>& blackPieces) {
  std::vector<sf::Vector2i> moves;
  const std::vector<Piece>& ownPieces = (bishop.color == Color::kWhite ? whitePieces : blackPieces);
  const std::vector<Piece>& enemyPieces = (bishop.color == Color::kWhite ? blackPieces : whitePieces);


  int directions[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
  for (int i = 0; i < 4; ++i) {
    int dx = directions[i][0];
    int dy = directions[i][1];
    sf::Vector2i pos = bishop.pos;

    while (true) {
      pos.x += dx;
      pos.y += dy;
      if (pos.x < 0 || pos.x >= 8 || pos.y < 0 || pos.y >= 8)
        break;
      if (isOccupied(pos, ownPieces))
        break;
      moves.push_back(pos);
      if (isOccupied(pos, enemyPieces))
        break;
    }
  }
  return moves;
}

std::vector<sf::Vector2i> getRookMoves(const Piece& rook,
                                       const std::vector<Piece>& whitePieces,
                                       const std::vector<Piece>& blackPieces) {
  std::vector<sf::Vector2i> moves;
  const std::vector<Piece>& ownPieces = (rook.color == Color::kWhite ? whitePieces : blackPieces);
  const std::vector<Piece>& enemyPieces = (rook.color == Color::kWhite ? blackPieces : whitePieces);


  int directions[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
  for (int i = 0; i < 4; ++i) {
    int dx = directions[i][0];
    int dy = directions[i][1];
    sf::Vector2i pos = rook.pos;
    while (true) {
      pos.x += dx;
      pos.y += dy;
      if (pos.x < 0 || pos.x >= 8 || pos.y < 0 || pos.y >= 8)
        break;
      if (isOccupied(pos, ownPieces))
        break;
      moves.push_back(pos);
      if (isOccupied(pos, enemyPieces))
        break;
    }
  }
  return moves;
}


std::vector<sf::Vector2i> getQueenMoves(const Piece& queen,
                                        const std::vector<Piece>& whitePieces,
                                        const std::vector<Piece>& blackPieces) {
  std::vector<sf::Vector2i> moves;
  const std::vector<Piece>& ownPieces = (queen.color == Color::kWhite ? whitePieces : blackPieces);
  const std::vector<Piece>& enemyPieces = (queen.color == Color::kWhite ? blackPieces : whitePieces);


  int directions[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1},
                           {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
  for (int i = 0; i < 8; ++i) {
    int dx = directions[i][0];
    int dy = directions[i][1];
    sf::Vector2i pos = queen.pos;
    while (true) {
      pos.x += dx;
      pos.y += dy;
      if (pos.x < 0 || pos.x >= 8 || pos.y < 0 || pos.y >= 8)
        break;
      if (isOccupied(pos, ownPieces))
        break;
      moves.push_back(pos);
      if (isOccupied(pos, enemyPieces))
        break;
    }
  }
  return moves;
}
std::vector<sf::Vector2i> getKingMoves(const Piece& king,
                                       const std::vector<Piece>& whitePieces,
                                       const std::vector<Piece>& blackPieces) {
  std::vector<sf::Vector2i> moves;
  const std::vector<Piece>& ownPieces = (king.color == Color::kWhite ? whitePieces : blackPieces);


  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      if (dx == 0 && dy == 0)
        continue;
      sf::Vector2i newPos = { king.pos.x + dx, king.pos.y + dy };
      if (newPos.x >= 0 && newPos.x < 8 && newPos.y >= 0 && newPos.y < 8) {
        if (!isOccupied(newPos, ownPieces))
          moves.push_back(newPos);
      }
    }
  }
  return moves;
}
void DrawPieces(sf::RenderWindow &window, const std::vector<Piece> &pieces, float tile_size) {
  for (const auto &piece : pieces) {

    sf::Sprite sprite = piece.sprite;


    sf::FloatRect bounds = sprite.getLocalBounds();
    float width = bounds.size.x;
    float height = bounds.size.y;

    sprite.setOrigin(sf::Vector2f(width / 2.0f ,height / 2.0f));

    float posX = piece.pos.x * tile_size + tile_size / 2;
    float posY = piece.pos.y * tile_size + tile_size / 2;
    sprite.setPosition(sf::Vector2f (posX, posY));
    window.draw(sprite);
  }
}


int main() {
  sf::RenderWindow window(sf::VideoMode({1280, 1280}), "Simple Chat");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);
  bool isOpen = true;
  sf::Clock deltaClock;
  Status status = Status::NOT_CONNECTED;
  std::vector<std::string> receivedMessages;
  bool winner_PA = false;
  bool winner_PB = false;


  sf::TcpSocket socket;
  socket.setBlocking(false);
  std::string serverAddress = "localhost";
  serverAddress.resize(50, 0);
  short portNumber = PORT_NUMBER;
  std::string sendMessage;
  sendMessage.resize(MAX_MESSAGE_LENGTH, 0);
  std::string receivedMessage;
  receivedMessage.resize(MAX_MESSAGE_LENGTH, 0);
  bool firstIT = true;

  //-------------------------------------------------------------------

  int window_size = window.getSize().x;
  auto tile_size = static_cast<float>(window_size / 8);
  float offset = 10.0f;
  sf::Vector2i selectedTileCoords = sf::Vector2i(-1, -1);
  sf::Vector2i lastSelectedTileCoords = sf::Vector2i(-1, -1);
  sf::Vector2f mousePos = sf::Vector2f(0, 0);
  bool selecting = true;
  std::vector<sf::Vector2f> positions;
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      positions.emplace_back(tile_size * x, tile_size * y);
    }
  }

  sf::Texture king_texture_w;
  sf::Texture queen_texture_w;
  sf::Texture rooks_texture_w;
  sf::Texture bishops_texture_w;
  sf::Texture knights_texture_w;
  sf::Texture pawns_texture_w;

  sf::Texture king_texture_b;
  sf::Texture queen_texture_b;
  sf::Texture rooks_texture_b;
  sf::Texture bishops_texture_b;
  sf::Texture knights_texture_b;
  sf::Texture pawns_texture_b;

  if (!king_texture_w.loadFromFile("data/tile019.png"))
    std::cerr << "Erreur de chargement de la texture du roi blanc !" << std::endl;
  if (!queen_texture_w.loadFromFile("data/tile018.png"))
    std::cerr << "Erreur de chargement de la texture de la reine blanche !" << std::endl;
  if (!rooks_texture_w.loadFromFile("data/tile015.png"))
    std::cerr << "Erreur de chargement de la texture des tours blanches !" << std::endl;
  if (!bishops_texture_w.loadFromFile("data/tile017.png"))
    std::cerr << "Erreur de chargement de la texture des fous blancs !" << std::endl;
  if (!knights_texture_w.loadFromFile("data/tile016.png"))
    std::cerr << "Erreur de chargement de la texture des cavaliers blancs !" << std::endl;
  if (!pawns_texture_w.loadFromFile("data/tile010.png"))
    std::cerr << "Erreur de chargement de la texture des pions blancs !" << std::endl;


  if (!king_texture_b.loadFromFile("data/tile004.png"))
    std::cerr << "Erreur de chargement de la texture du roi noir !" << std::endl;
  if (!queen_texture_b.loadFromFile("data/tile003.png"))
    std::cerr << "Erreur de chargement de la texture de la reine noire !" << std::endl;
  if (!rooks_texture_b.loadFromFile("data/tile000.png"))
    std::cerr << "Erreur de chargement de la texture des tours noires !" << std::endl;
  if (!bishops_texture_b.loadFromFile("data/tile002.png"))
    std::cerr << "Erreur de chargement de la texture des fous noirs !" << std::endl;
  if (!knights_texture_b.loadFromFile("data/tile001.png"))
    std::cerr << "Erreur de chargement de la texture des cavaliers noirs !" << std::endl;
  if (!pawns_texture_b.loadFromFile("data/tile005.png"))
    std::cerr << "Erreur de chargement de la texture des pions noirs !" << std::endl;


  sf::Sprite pawns_w(pawns_texture_w);
  pawns_w.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite king_w(king_texture_w);
  king_w.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite queen_w(queen_texture_w);
  queen_w.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite rooks_w(rooks_texture_w);
  rooks_w.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite bishops_w(bishops_texture_w);
  bishops_w.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite knights_w(knights_texture_w);
  knights_w.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite pawns_b(pawns_texture_b);
  pawns_b.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite king_b(king_texture_b);
  king_b.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite queen_b(queen_texture_b);
  queen_b.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite rooks_b(rooks_texture_b);
  rooks_b.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite bishops_b(bishops_texture_b);
  bishops_b.setScale(sf::Vector2f (2.5,2.5));
  sf::Sprite knights_b(knights_texture_b);
  knights_b.setScale(sf::Vector2f (2.5,2.5));

  std::vector<Piece> whitePieces;
  std::vector<Piece> blackPieces;

  Color local_player = Color::kNuLL;
  std::vector<Piece> *currentPiecesPlayer1;
  std::vector<Piece> *currentPiecesPlayer2;

  whitePieces.push_back({PieceType::Rook,   Color::kWhite, sf::Vector2i(0, 7), rooks_w});
  whitePieces.push_back({PieceType::Knight, Color::kWhite, sf::Vector2i(1, 7), knights_w});
  whitePieces.push_back({PieceType::Bishop, Color::kWhite, sf::Vector2i(2, 7), bishops_w});
  whitePieces.push_back({PieceType::Queen,  Color::kWhite, sf::Vector2i(3, 7), queen_w});
  whitePieces.push_back({PieceType::King,   Color::kWhite, sf::Vector2i(4, 7), king_w});
  whitePieces.push_back({PieceType::Bishop, Color::kWhite, sf::Vector2i(5, 7), bishops_w});
  whitePieces.push_back({PieceType::Knight, Color::kWhite, sf::Vector2i(6, 7), knights_w});
  whitePieces.push_back({PieceType::Rook,   Color::kWhite, sf::Vector2i(7, 7), rooks_w});

  for (int i = 0; i < 8; i++) {
    whitePieces.push_back({PieceType::Pawn, Color::kWhite, sf::Vector2i(i, 6), pawns_w});
  }


  blackPieces.push_back({PieceType::Rook,   Color::kBlack, sf::Vector2i(0, 0), rooks_b});
  blackPieces.push_back({PieceType::Knight, Color::kBlack, sf::Vector2i(1, 0), knights_b});
  blackPieces.push_back({PieceType::Bishop, Color::kBlack, sf::Vector2i(2, 0), bishops_b});
  blackPieces.push_back({PieceType::Queen,  Color::kBlack, sf::Vector2i(3, 0), queen_b});
  blackPieces.push_back({PieceType::King,   Color::kBlack, sf::Vector2i(4, 0), king_b});
  blackPieces.push_back({PieceType::Bishop, Color::kBlack, sf::Vector2i(5, 0), bishops_b});
  blackPieces.push_back({PieceType::Knight, Color::kBlack, sf::Vector2i(6, 0), knights_b});
  blackPieces.push_back({PieceType::Rook,   Color::kBlack, sf::Vector2i(7, 0), rooks_b});
  for (int i = 0; i < 8; i++) {
    blackPieces.push_back({PieceType::Pawn, Color::kBlack, sf::Vector2i(i, 1), pawns_b});
  }

  std::vector<sf::Vector2i> optionsPos;

  sf::RectangleShape tiles;
  tiles.setSize(sf::Vector2f(tile_size, tile_size));

  sf::RectangleShape selectedTile;
  selectedTile.setSize(sf::Vector2f(tile_size, tile_size));
  selectedTile.setFillColor(sf::Color(242, 242, 167, 255));

  sf::CircleShape options;
  options.setRadius(tile_size / 6);
  options.setFillColor(sf::Color(87, 84, 82, 100));

  while (isOpen) {

    if (status == Status::NOT_CONNECTED) {
      if (auto address = sf::IpAddress::resolve(serverAddress)) {
        socket.setBlocking(true);
        const auto connectionStatus = socket.connect(address.value(), portNumber);
        switch (connectionStatus) {
          case sf::Socket::Status::Done:
            status = Status::CONNECTED;
            std::cout << "Connected automatically to " << serverAddress << ":" << portNumber << std::endl;
            break;
          case sf::Socket::Status::NotReady:
            std::cerr << "Socket not ready, retrying..." << std::endl;
            break;
          case sf::Socket::Status::Partial:
            std::cerr << "Partial connection, retrying..." << std::endl;
            break;
          case sf::Socket::Status::Disconnected:
            std::cerr << "Socket disconnected, retrying..." << std::endl;
            break;
          case sf::Socket::Status::Error:
            std::cerr << "Socket error, retrying..." << std::endl;
            break;
        }
        socket.setBlocking(false);
      } else {
        std::cerr << "Invalid server address: " << serverAddress << std::endl;
      }
    }



    while (const std::optional event = window.pollEvent()) {
      ImGui::SFML::ProcessEvent(window, *event);

      if (event->is<sf::Event::Closed>()) {
        isOpen = false;
      }

      if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
          window.close();
        }
      }

      if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
        selectedTileCoords = sf::Vector2i(floor(static_cast<int>(mousePos.x) / tile_size),
                                          floor(static_cast<int>(mousePos.y) / tile_size));

        if (!selecting) {
          lastSelectedTileCoords = selectedTileCoords;
          selecting = true;
          optionsPos.clear();

          for (const auto &piece : *currentPiecesPlayer1) {
            if (selectedTileCoords == piece.pos) {
              switch (piece.type) {
                case PieceType::Pawn:optionsPos = getPawnMoves(piece, whitePieces, blackPieces);
                  break;
                case PieceType::Knight:optionsPos = getKnightMoves(piece, whitePieces, blackPieces);
                  break;
                case PieceType::Bishop:optionsPos = getBishopMoves(piece, whitePieces, blackPieces);
                  break;
                case PieceType::King:optionsPos = getKingMoves(piece, whitePieces, blackPieces);
                  break;
                case PieceType::Queen:optionsPos = getQueenMoves(piece, whitePieces, blackPieces);
                  break;
                case PieceType::Rook:optionsPos = getRookMoves(piece, whitePieces, blackPieces);
                  break;
              }
              break;
            }
          }
        } else {
          bool moveFound = false;
          for (const auto &pos : optionsPos) {
            if (selectedTileCoords == pos) {
              for (auto &piece : *currentPiecesPlayer1) {
                if (piece.pos == lastSelectedTileCoords) {
                  //std::cout << "Déplacement demandé !" << std::endl;


                  std::ostringstream messageStream;
                  messageStream << "MOVE|"
                                << static_cast<int>(piece.type) << "|"
                                << piece.pos.x << "," << piece.pos.y << "|"
                                << selectedTileCoords.x << "," << selectedTileCoords.y;

                  std::string message = messageStream.str();


                  if (socket.send(message.c_str(), message.size()) != sf::Socket::Status::Done) {
                    std::cerr << "Error" << std::endl;
                  } else {
                    std::cout <<  message << std::endl;
                  }
                  break;
                }
              }
              moveFound = true;
              optionsPos.clear();
              selecting = false;
              break;
            }
          }

          if (!moveFound) {

            bool foundNewSelection = false;
            for (const auto &piece : *currentPiecesPlayer1) {
              if (piece.pos == selectedTileCoords) {
                lastSelectedTileCoords = selectedTileCoords;
                optionsPos.clear();
                switch (piece.type) {
                  case PieceType::Pawn:optionsPos = getPawnMoves(piece, whitePieces, blackPieces);
                    break;
                  case PieceType::Knight:optionsPos = getKnightMoves(piece, whitePieces, blackPieces);
                    break;
                  case PieceType::Bishop:optionsPos = getBishopMoves(piece, whitePieces, blackPieces);
                    break;
                  case PieceType::King:optionsPos = getKingMoves(piece, whitePieces, blackPieces);
                    break;
                  case PieceType::Queen:optionsPos = getQueenMoves(piece, whitePieces, blackPieces);
                    break;
                  case PieceType::Rook:optionsPos = getRookMoves(piece, whitePieces, blackPieces);
                    break;
                }
                foundNewSelection = true;
                break;
              }
            }

            if (!foundNewSelection) {
              selecting = false;
              optionsPos.clear();
            }
          }
        }
      }
    }



  if (status == Status::CONNECTED) {

    std::string message;
    //std::cout << "Message reçu : " << message << std::endl;
    message.resize(MAX_MESSAGE_LENGTH, 0);
    size_t actualLength;
    sf::TcpSocket::Status receiveStatus = socket.receive(message.data(), MAX_MESSAGE_LENGTH, actualLength);

    if (receiveStatus == sf::Socket::Status::Done) {

      if (message.find("ROLE") == 0) {
        size_t separatorPos = message.find('|');
        if (separatorPos != std::string::npos) {
          std::string role = message.substr(separatorPos + 1);
          if (role.find("PA") != std::string::npos) {
            local_player = Color::kWhite;
            currentPiecesPlayer1 = &whitePieces;
            //std::cout << "Local player color: " << static_cast<int>(local_player) << std::endl;
          }
          else if (role.find("PB") != std::string::npos) {
            local_player = Color::kBlack;
            currentPiecesPlayer1 = &blackPieces;
            //std::cout << "Local player color: " << static_cast<int>(local_player) << std::endl;
          }
        } else {
          std::cerr << "mesage null" << std::endl;
        }

      }
      if (message.find("MOVE") == 0) {
        std::istringstream iss(message);
        std::string token;


        std::getline(iss, token, '|');  // token == "MOVE"


        std::string role;
        std::getline(iss, role, '|');


        std::getline(iss, token, '|');
        int pieceType = std::stoi(token);


        std::getline(iss, token, '|');
        size_t commaPos = token.find(',');
        int oldX = std::stoi(token.substr(0, commaPos));
        int oldY = std::stoi(token.substr(commaPos + 1));


        std::getline(iss, token, '|');
        commaPos = token.find(',');
        int newX = std::stoi(token.substr(0, commaPos));
        int newY = std::stoi(token.substr(commaPos + 1));

//        std::cout << "Mouvement du joueur " << role
//                  << " : pièce de type " << pieceType
//                  << " de (" << oldX << ", " << oldY << ") vers ("
//                  << newX << ", " << newY << ")" << std::endl;


        if (role == "PA") {
          MovePiece(whitePieces, sf::Vector2i(oldX, oldY), sf::Vector2i(newX, newY));
        }
        else if (role == "PB") {
          MovePiece(blackPieces, sf::Vector2i(oldX, oldY), sf::Vector2i(newX, newY));
        }
      }
      if (message.find("CAPTURE") == 0) {

        std::istringstream iss(message);
        std::string token;


        std::getline(iss, token, '|');


        std::string capRole;
        std::getline(iss, capRole, '|');


        std::getline(iss, token, '|');
        int capturedType = std::stoi(token);


        std::getline(iss, token, '|');
        size_t commaPos = token.find(',');
        int capX = std::stoi(token.substr(0, commaPos));
        int capY = std::stoi(token.substr(commaPos + 1));

//        std::cout << "Capture d'une pièce de type " << capturedType
//                  << " à la position (" << capX << ", " << capY << ") appartenant à "
//                  << capRole << std::endl;


        if (capRole == "PA") {
          RemovePiece(whitePieces, sf::Vector2i(capX, capY));
        } else if (capRole == "PB") {
          RemovePiece(blackPieces, sf::Vector2i(capX, capY));
        }

      }
      if (message.find("CHECKMATE") == 0) {
        std::istringstream iss(message);
        std::string token;


        std::getline(iss, token, '|');


        std::string winnerRole;
        std::getline(iss, winnerRole, '|');

        std::cout << "Error : " << winnerRole << std::endl;

        if (winnerRole == "PA") {
          winner_PA = true;
        } else if (winnerRole == "PB") {
          winner_PB = true;
        }
      }

    }
    if (socket.getLocalPort() == 0) {
      status = Status::NOT_CONNECTED;
    }

  }
  ImGui::SFML::Update(window, deltaClock.restart());
  auto [x, y] = window.getSize();
  ImGui::SetNextWindowSize({(float) x, (float) y}, ImGuiCond_Always);
  ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
  ImGui::Begin("Simple Chat", nullptr, ImGuiWindowFlags_NoTitleBar);
  switch (status) {
    case Status::NOT_CONNECTED: {
      ImGui::InputText("Host Address", serverAddress.data(), serverAddress.size());
      ImGui::SameLine();
      ImGui::Text("%hd", portNumber);
      if (ImGui::Button("Connect")) {
        if (auto address = sf::IpAddress::resolve(serverAddress)) {
          socket.setBlocking(true);
          const auto connectionStatus = socket.connect(address.value(), portNumber);
          switch (connectionStatus) {
            case sf::Socket::Status::Done: status = Status::CONNECTED;
              break;
            case sf::Socket::Status::NotReady: std::cerr << "Socket not ready\n";
              break;
            case sf::Socket::Status::Partial: std::cerr << "Partial\n";
              break;
            case sf::Socket::Status::Disconnected: std::cerr << "Socket disconnected\n";
              break;
            case sf::Socket::Status::Error: std::cerr << "Socket error\n";
              break;
          }
          socket.setBlocking(false);
        }
      }
      break;
    }
    case Status::CONNECTED: {
      ImGui::InputText("Message", sendMessage.data(), MAX_MESSAGE_LENGTH);
      if (ImGui::Button("Send")) {
        size_t dataSent;
        sf::TcpSocket::Status sendStatus;
        do {
          sendStatus = socket.send(sendMessage.data(), MAX_MESSAGE_LENGTH, dataSent);
        } while (sendStatus == sf::Socket::Status::Partial);
      }
      for (const auto &message : receivedMessages) {
        ImGui::Text("Received message: %s", message.data());
      }
      break;
    }
  }

  ImGui::End();

  window.clear();

  ImGui::SFML::Render(window);
  int i = 0;
  for (sf::Vector2f pos : positions) {
    if (i % 2 == 0) {
      tiles.setFillColor(sf::Color(235, 236, 208, 255));
    } else {
      tiles.setFillColor(sf::Color(119, 149, 86, 255));
    }
    i++;
    if (i % 9 == 8) {
      i++;
    }
    tiles.setPosition(pos);
    window.draw(tiles);
  }

    if (selecting) {
      selectedTile.setPosition(sf::Vector2f(selectedTileCoords.x * tile_size, selectedTileCoords.y * tile_size));
      window.draw(selectedTile);
    }

    DrawPieces(window, whitePieces, tile_size);
    DrawPieces(window, blackPieces, tile_size);

    if (selecting) {
    for (auto &pos : optionsPos) {
      options.setPosition(sf::Vector2f(pos.x * tile_size + tile_size / 3, pos.y * tile_size + tile_size / 3));
      window.draw(options);
    }
  }

  window.display();
  firstIT = false;

    if (winner_PA || winner_PB) {
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
      ImGui::SetNextWindowPos(ImVec2(window.getSize().x * 0.5f, window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
      ImGui::Begin("Overlay", nullptr,
                   ImGuiWindowFlags_NoDecoration |
                       ImGuiWindowFlags_NoInputs |
                       ImGuiWindowFlags_AlwaysAutoResize);

      if (winner_PA)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Échec et mat ! Joueur 1 gagne !");
      else if (winner_PB)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Échec et mat ! Joueur 2 gagne !");
      else
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "Échec et mat !");

      ImGui::End();
      ImGui::PopStyleColor();
    }

}

ImGui::SFML::Shutdown();
}

