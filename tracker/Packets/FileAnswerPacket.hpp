#ifndef FILE_ANWSER_PACKET_HPP
#define FILE_ANWSER_PACKET_HPP


#include <vector>
#include <string>
#include "IPacket.hpp"

/**
 * Organisation d'un FileAnswer :
 * ------------------------------------------------
 * |opcode|filename|assocNb|tableau d'associations|
 * ------------------------------------------------
 * | 4    | 100    | 4     | taille d'assoc * 8   | tailles en octets
 * ------------------------------------------------
 *    Association :
 *    --------------------------
 *    | nbPartition | ipClient |
 *    --------------------------
 *    | 4           | 60 ?     |
 *    --------------------------
 * Taille d'un FileAnswer : 620 octets
 */
class FileAnswerPacket : public IPacket {
    public:
        static const int MAX_FILENAME_SIZE = 100;
        static const int MAX_ASSOC_NUMBER = 8;

        struct Association {
            int partition;
            char ipClient[60];
        };

        /**
         * Construit un paquet à partir de ces paramétres.
         */
        FileAnswerPacket(std::string filename, std::vector<Association> assoc);

        /**
         * Reconstruit un paquet à partir des données au format réseau.
         */
        FileAnswerPacket(char* data, int size);

        ~FileAnswerPacket();

        /**
         * Renvoie l'opcode du packet.
         */
        unsigned int getOpcode();

        /**
         * Renvoie la taille du paquet.
         */
        int getSize();

        /**
         * Renvoie le paquet sous la forme d'un tableau de char dans le format réseau.
         * Note : Les 4 premiers octets sont un entier représentant un opcode.
         */
        char* toData();

        /**
         * Execute la commande associée à ce paquet.
         * @param adresse L'adresse de la personne ayant envoyé ce paquet.
         */
        void exec(std::string adresse);

    private:
        std::string mFileName;
        std::vector<Association> mAssoc;
};

#endif // IPACKET_HPP

