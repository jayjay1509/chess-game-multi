#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/SocketSelector.hpp>

#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <ranges>
#include <map>
#include <sstream>
#include <optional>
#include <cmath> // pour std::abs
#include <thread>

#include "const.h"
#include "SFML/System/Vector2.hpp"

enum class Color { kWhite, kBlack, kNone };

enum class PieceType { King, Queen, Rook, Bishop, Knight, Pawn };

struct Piece {
  PieceType type;
  Color color;
  sf::Vector2i pos;
};

bool isMoveValid(const Piece& piece, const sf::Vector2i& targetPos,
                 const std::array<std::array<std::optional<Piece>, 8>, 8>& board) {

  if (targetPos.x < 0 || targetPos.x >= 8 || targetPos.y < 0 || targetPos.y >= 8)
    return false;


  const auto& targetCell = board[targetPos.y][targetPos.x];
  if (targetCell.has_value() && targetCell->color == piece.color)
    return false;

  int dx = std::abs(piece.pos.x - targetPos.x);
  int dy = std::abs(piece.pos.y - targetPos.y);

  switch (piece.type) {
    case PieceType::King:
      return dx <= 1 && dy <= 1;

    case PieceType::Queen:

      if (dx == dy && dx > 0) {
        int stepX = (targetPos.x > piece.pos.x) ? 1 : -1;
        int stepY = (targetPos.y > piece.pos.y) ? 1 : -1;
        int x = piece.pos.x + stepX;
        int y = piece.pos.y + stepY;
        while (x != targetPos.x && y != targetPos.y) {
          if (board[y][x].has_value())
            return false;
          x += stepX;
          y += stepY;
        }
        return true;
      }

      if ((dx == 0 && dy > 0) || (dy == 0 && dx > 0)) {
        int stepX = (targetPos.x > piece.pos.x) ? 1 : (targetPos.x < piece.pos.x) ? -1 : 0;
        int stepY = (targetPos.y > piece.pos.y) ? 1 : (targetPos.y < piece.pos.y) ? -1 : 0;
        int x = piece.pos.x + stepX;
        int y = piece.pos.y + stepY;
        while (x != targetPos.x || y != targetPos.y) {
          if (board[y][x].has_value())
            return false;
          x += stepX;
          y += stepY;
        }
        return true;
      }
      return false;

    case PieceType::Rook:
      if ((dx == 0 && dy > 0) || (dy == 0 && dx > 0)) {
        int stepX = (targetPos.x > piece.pos.x) ? 1 : (targetPos.x < piece.pos.x) ? -1 : 0;
        int stepY = (targetPos.y > piece.pos.y) ? 1 : (targetPos.y < piece.pos.y) ? -1 : 0;
        int x = piece.pos.x + stepX;
        int y = piece.pos.y + stepY;
        while (x != targetPos.x || y != targetPos.y) {
          if (board[y][x].has_value())
            return false;
          x += stepX;
          y += stepY;
        }
        return true;
      }
      return false;

    case PieceType::Bishop:
      if (dx == dy && dx > 0) {
        int stepX = (targetPos.x > piece.pos.x) ? 1 : -1;
        int stepY = (targetPos.y > piece.pos.y) ? 1 : -1;
        int x = piece.pos.x + stepX;
        int y = piece.pos.y + stepY;
        while (x != targetPos.x && y != targetPos.y) {
          if (board[y][x].has_value())
            return false;
          x += stepX;
          y += stepY;
        }
        return true;
      }
      return false;

    case PieceType::Knight:
      return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);

    case PieceType::Pawn:
      if (piece.color == Color::kWhite) {

        if (targetPos.y == piece.pos.y - 1 && dx == 0 && !targetCell.has_value())
          return true;

        if (piece.pos.y == 6 && targetPos.y == piece.pos.y - 2 && dx == 0 &&
            !board[piece.pos.y - 1][piece.pos.x].has_value() && !targetCell.has_value())
          return true;

        if (targetPos.y == piece.pos.y - 1 && dx == 1 && targetCell.has_value() &&
            targetCell->color == Color::kBlack)
          return true;
      } else if (piece.color == Color::kBlack) {
        if (targetPos.y == piece.pos.y + 1 && dx == 0 && !targetCell.has_value())
          return true;
        if (piece.pos.y == 1 && targetPos.y == piece.pos.y + 2 && dx == 0 &&
            !board[piece.pos.y + 1][piece.pos.x].has_value() && !targetCell.has_value())
          return true;
        if (targetPos.y == piece.pos.y + 1 && dx == 1 && targetCell.has_value() &&
            targetCell->color == Color::kWhite)
          return true;
      }
      return false;

    default:
      return false;
  }
}

bool isKingInCheck(Color kingColor,
                   const std::array<std::array<std::optional<Piece>, 8>, 8>& board) {
  sf::Vector2i kingPos(-1, -1);

  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      if (board[y][x].has_value()) {
        const Piece& p = board[y][x].value();
        if (p.type == PieceType::King && p.color == kingColor) {
          kingPos = sf::Vector2i(x, y);
          break;
        }
      }
    }
    if (kingPos.x != -1)
      break;
  }
  if (kingPos.x == -1)
    return false;


  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      if (board[y][x].has_value()) {
        const Piece& p = board[y][x].value();
        if (p.color != kingColor) {
          if (isMoveValid(p, kingPos, board)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool isCheckmate(Color playerColor,
                 const std::array<std::array<std::optional<Piece>, 8>, 8>& board) {

  if (!isKingInCheck(playerColor, board)) {
    return false;
  }


  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {

      if (board[y][x].has_value() && board[y][x]->color == playerColor) {
        Piece currentPiece = board[y][x].value();

        for (int ty = 0; ty < 8; ++ty) {
          for (int tx = 0; tx < 8; ++tx) {
            sf::Vector2i targetPos(tx, ty);

            if (isMoveValid(currentPiece, targetPos, board)) {

              auto boardCopy = board;

              boardCopy[currentPiece.pos.y][currentPiece.pos.x].reset();

              Piece simulatedPiece = currentPiece;
              simulatedPiece.pos = targetPos;
              boardCopy[ty][tx] = simulatedPiece;

              if (!isKingInCheck(playerColor, boardCopy)) {
                return false;
              }
            }
          }
        }
      }
    }
  }

  return true;
}


int main()
{

  std::vector<Piece> PAPieces;
  std::vector<Piece> PBPieces;

  PAPieces.push_back({PieceType::Rook,   Color::kWhite, sf::Vector2i(0, 7)});
  PAPieces.push_back({PieceType::Knight, Color::kWhite, sf::Vector2i(1, 7)});
  PAPieces.push_back({PieceType::Bishop, Color::kWhite, sf::Vector2i(2, 7)});
  PAPieces.push_back({PieceType::Queen,  Color::kWhite, sf::Vector2i(3, 7)});
  PAPieces.push_back({PieceType::King,   Color::kWhite, sf::Vector2i(4, 7)});
  PAPieces.push_back({PieceType::Bishop, Color::kWhite, sf::Vector2i(5, 7)});
  PAPieces.push_back({PieceType::Knight, Color::kWhite, sf::Vector2i(6, 7)});
  PAPieces.push_back({PieceType::Rook,   Color::kWhite, sf::Vector2i(7, 7)});

  for (int i = 0; i < 8; i++) {
    PAPieces.push_back({PieceType::Pawn, Color::kWhite, sf::Vector2i(i, 6)});
  }

  PBPieces.push_back({PieceType::Rook,   Color::kBlack, sf::Vector2i(0, 0)});
  PBPieces.push_back({PieceType::Knight, Color::kBlack, sf::Vector2i(1, 0)});
  PBPieces.push_back({PieceType::Bishop, Color::kBlack, sf::Vector2i(2, 0)});
  PBPieces.push_back({PieceType::Queen,  Color::kBlack, sf::Vector2i(3, 0)});
  PBPieces.push_back({PieceType::King,   Color::kBlack, sf::Vector2i(4, 0)});
  PBPieces.push_back({PieceType::Bishop, Color::kBlack, sf::Vector2i(5, 0)});
  PBPieces.push_back({PieceType::Knight, Color::kBlack, sf::Vector2i(6, 0)});
  PBPieces.push_back({PieceType::Rook,   Color::kBlack, sf::Vector2i(7, 0)});
  for (int i = 0; i < 8; i++) {
    PBPieces.push_back({PieceType::Pawn, Color::kBlack, sf::Vector2i(i, 1)});
  }


  std::array<std::array<std::optional<Piece>, 8>, 8> board;
  // On initialise toutes les cases à vide
  for (auto& row : board)
    for (auto& cell : row)
      cell.reset();

  // Placement des pièces sur le plateau
  for (const Piece& p : PAPieces) {
    board[p.pos.y][p.pos.x] = p;
  }
  for (const Piece& p : PBPieces) {
    board[p.pos.y][p.pos.x] = p;
  }

  //-----------------------------------------------------------------------
  std::vector<std::unique_ptr<sf::TcpSocket>> sockets;
  sockets.reserve(15);
  sf::TcpListener listener;
  sf::SocketSelector socketSelector;
  std::map<sf::TcpSocket*, std::string> playerRoles;

  listener.setBlocking(false);
  const auto listenerStatus = listener.listen(PORT_NUMBER);
  switch(listenerStatus)
  {
    case sf::Socket::Status::Done:
      break;
    default:
      std::cerr << "Error while listening\n";
      return EXIT_FAILURE;
  }

  int playerCount = 0;



  Color currentTurn = Color::kWhite;


  while (true)
  {
    for (auto& socket : sockets)
    {
      if (socket == nullptr)
        continue;
      if (socket->getLocalPort() == 0)
      {
        socket = nullptr;
      }
    }
    {
      sf::TcpSocket socket;
      socket.setBlocking(false);
      const auto newSocketStatus = listener.accept(socket);
      if(newSocketStatus == sf::Socket::Status::Done)
      {
        auto newSocket = std::make_unique<sf::TcpSocket>(std::move(socket));
        socketSelector.add(*newSocket);

        std::string roleMessage;
        if (playerCount == 0) {
          roleMessage = "ROLE|PA";
          playerRoles[newSocket.get()] = "PA";
          playerCount++;
        } else if (playerCount == 1) {
          roleMessage = "ROLE|PB";
          playerRoles[newSocket.get()] = "PB";
          playerCount++;
        }


        size_t sentDataCount;
        sf::TcpSocket::Status sendStatus;
        do
        {
          sendStatus = newSocket->send(roleMessage.data(), roleMessage.size(), sentDataCount);
        } while (sendStatus == sf::Socket::Status::Partial);

        auto it = std::ranges::find_if(sockets, [](auto& local_socket)
        {
          return local_socket == nullptr;
        });
        if (it != sockets.end())
        {
          *it = std::move(newSocket);
        }
        else
        {
          sockets.push_back(std::move(newSocket));
        }
      }
    }

    if(socketSelector.wait(sf::milliseconds(100)))
    {
      for(auto& socket: sockets)
      {
        if(socket == nullptr)
          continue;
        if(socketSelector.isReady(*socket))
        {
          std::string message;
          message.resize(MAX_MESSAGE_LENGTH, 0);
          size_t actualLength;

          sf::TcpSocket::Status receiveStatus = socket->receive(message.data(), MAX_MESSAGE_LENGTH, actualLength);
          switch(receiveStatus)
          {
            case sf::Socket::Status::Done:
            {
              std::string player;
              if (playerRoles.contains(socket.get()))
              {
                player = playerRoles[socket.get()];
              }
              else
              {
                std::cout << "Error :" <<  message << std::endl;
                break;
              }

              std::stringstream ss(message);
              std::string token;


              std::getline(ss, token, '|'); // attend "MOVE"
              if (token == "MOVE") {
                std::getline(ss, token, '|');
                int pieceType = std::stoi(token);

                std::getline(ss, token, '|');
                std::string piecePosStr = token;
                size_t commaPos = piecePosStr.find(',');
                int piecePosX = std::stoi(piecePosStr.substr(0, commaPos));
                int piecePosY = std::stoi(piecePosStr.substr(commaPos + 1));

                std::getline(ss, token, '|');
                std::string newTileCoordsStr = token;
                commaPos = newTileCoordsStr.find(',');
                int newTileX = std::stoi(newTileCoordsStr.substr(0, commaPos));
                int newTileY = std::stoi(newTileCoordsStr.substr(commaPos + 1));


                Color playerColor = (player == "PA") ? Color::kWhite : Color::kBlack;

                if (playerCount != 2) {
                  std::cerr << "Error\n";
                  break;
                }


                if ((currentTurn == Color::kWhite && playerColor != Color::kWhite) ||
                    (currentTurn == Color::kBlack && playerColor != Color::kBlack)) {
                  std::cerr << "Error\n";
                  break;
                }


                if (piecePosX < 0 || piecePosX >= 8 || piecePosY < 0 || piecePosY >= 8 ||
                    !board[piecePosY][piecePosX].has_value()) {
                  std::cerr  << piecePosX << ", " << piecePosY << ")\n";
                  break;
                }

                Piece movingPiece = board[piecePosY][piecePosX].value();


                if (movingPiece.color != playerColor) {
                  std::cerr << "Error\n";
                  break;
                }




                if (isMoveValid(movingPiece, sf::Vector2i(newTileX, newTileY), board)) {



                  auto boardCopy = board;


                  boardCopy[piecePosY][piecePosX].reset();


                  Piece simulatedPiece = movingPiece;
                  simulatedPiece.pos = sf::Vector2i(newTileX, newTileY);
                  boardCopy[newTileY][newTileX] = simulatedPiece;


                  if (isKingInCheck(playerColor, boardCopy)) {
                    std::cerr << "Error" << std::endl;
                    break;
                  }


                  std::optional<Piece> capturedPiece;
                  if (board[newTileY][newTileX].has_value()) {
                    capturedPiece = board[newTileY][newTileX];
                  }


                  board[piecePosY][piecePosX].reset();


                  movingPiece.pos = sf::Vector2i(newTileX, newTileY);
                  board[newTileY][newTileX] = movingPiece;

//                  std::cout << "Mouvement effectué de (" << piecePosX << ", " << piecePosY << ") vers ("
//                            << newTileX << ", " << newTileY << ")\n";


                  std::string moveMessage = "MOVE|";
                  moveMessage += player + "|";  // 'player' vaut "PA" ou "PB"
                  moveMessage += std::to_string(static_cast<int>(movingPiece.type)) + "|";
                  moveMessage += std::to_string(piecePosX) + "," + std::to_string(piecePosY) + "|";
                  moveMessage += std::to_string(newTileX) + "," + std::to_string(newTileY);



                  for (auto& client : sockets) {
                    if (client != nullptr) {
                      size_t sentBytes = 0;
                      sf::TcpSocket::Status status;
                      do {
                        status = client->send(moveMessage.data(), moveMessage.size(), sentBytes);
                      } while (status == sf::TcpSocket::Status::Partial);
                      if (status != sf::TcpSocket::Status::Done) {
                        std::cerr << "Error\n";
                      }
                    }
                  }
                  Color opponentColor = (playerColor == Color::kWhite) ? Color::kBlack : Color::kWhite;
                  if (isCheckmate(opponentColor, board)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    std::string checkmateMessage = "CHECKMATE|";
                    checkmateMessage += (player == "PA") ? "PA" : "PB";
                    for (auto& client : sockets) {
                      if (client != nullptr) {
                        size_t sentBytes = 0;
                        sf::TcpSocket::Status status;
                        do {
                          status = client->send(checkmateMessage.data(), checkmateMessage.size(), sentBytes);
                          std::cout  << checkmateMessage << std::endl;
                        } while (status == sf::TcpSocket::Status::Partial);
                        if (status != sf::TcpSocket::Status::Done) {
                          std::cerr << "Error\n";
                        }
                      }
                    }
                    std::cout << "echec"  << ")\n";
                  }


                  currentTurn = (currentTurn == Color::kWhite) ? Color::kBlack : Color::kWhite;

                  // Si une pièce a été capturée, envoie aussi un message de capture
                  if (capturedPiece.has_value()) {


                    //uhmmmmmmm not good but send 2 mesage in same frame note work

                    std::this_thread::sleep_for(std::chrono::milliseconds(50));

                    // Construction du message de capture
                    std::string capRole = (capturedPiece->color == Color::kWhite) ? "PA" : "PB";
                    std::string captureMessage = "CAPTURE|";
                    captureMessage += capRole + "|";
                    captureMessage += std::to_string(static_cast<int>(capturedPiece->type)) + "|";
                    captureMessage += std::to_string(capturedPiece->pos.x) + "," + std::to_string(capturedPiece->pos.y);


                    // Envoi du message CAPTURE à tous les clients
                    for (auto& client : sockets) {
                      if (client != nullptr) {
                        size_t sentBytes = 0;
                        sf::TcpSocket::Status status;
                        do {
                          status = client->send(captureMessage.data(), captureMessage.size(), sentBytes);
                          std::cout  << captureMessage << std::endl;
                        } while (status == sf::TcpSocket::Status::Partial);
                        if (status != sf::TcpSocket::Status::Done) {
                          std::cerr << "Error\n";
                        }
                      }
                    }
                  }
                } else {
                  std::cerr << "Error\n";
                }
              }

              break;
            }
            case sf::TcpSocket::Status::Partial:
            {
              std::cerr << "Partial received...\n";
              break;
            }
            case sf::TcpSocket::Status::Error:
            {
              std::cerr << "Error receiving\n";
              break;
            }
            case sf::TcpSocket::Status::NotReady:
            {
              std::cerr << "Not ready on received\n";
              break;
            }
          }
        }
      }
    }
  }
}
