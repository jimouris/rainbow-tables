#include <iostream>
#include <fstream>
#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    #include "blake.h"
    #include "createRT.h"
#ifdef __cplusplus
}
#endif

using namespace std;

uint8_t* string2uint8_t(string str, size_t len) {
    uint8_t *ret = (uint8_t*) calloc((len/2)+1, sizeof(uint8_t));
    char *t = &str[0];
    for (int i = 0, j = 0 ; i < len ; i++) {
        uint8_t tmp;
        switch (t[i]) {
            case 'a': tmp = 10; break;
            case 'b': tmp = 11; break;
            case 'c': tmp = 12; break;
            case 'd': tmp = 13; break;
            case 'e': tmp = 14; break;
            case 'f': tmp = 15; break;
            default: tmp = t[i] - '0';
        }
        if (i % 2) {
            ret[j] += tmp;
            j++;
        } else {
            ret[j] += tmp*16;
        }
    }
    ret[len/2] = '\0';
    return ret;
}

string uint8_t2string(uint8_t * h_uint8, size_t len) {
    char *str = (char *) malloc((len+1) * sizeof(char));
    str[len] = '\0';
    char *strptr = str;
    for (int j = 0; j < len/2; j++) {
        sprintf(strptr, "%02x", h_uint8[j]);
        strptr += 2;
    }
    string h(str);
    free(str);
    return h;
}

// void search_chain(string first_pass_str, size_t chain_len, string hass2find) {
//     uint8_t *h_uint8 = (uint8_t *) malloc(33 * sizeof(uint8_t));
//     char *pass = (char *) malloc(7 * sizeof(char));
//     char *tmp = &first_pass_str[0];
//     memcpy(pass, tmp, 7);
//     pass[6] = '\0';
//     blake256_hash(h_uint8, (uint8_t *) pass, 6);
//     string h = uint8_t2string(h_uint8, 64);
//     size_t i = 1;
//     while (hass2find.compare(h) != 0) {
//         free(pass);
//         uint8_t *hash = string2uint8_t(h, 64);
//         pass = reduce(hash, i);
//         blake256_hash(h_uint8, (uint8_t *) pass, 6);
//         h = uint8_t2string(h_uint8, 64);
//         if (i >= CHAIN_LEN) {
//             return ;
//         }
//         i++;
//     }
//     cout << "\n\nEUREKA!!!!!\n" << hass2find << " : " << pass << "\n\n" ;
// };

void search_chain(string first_pass_str, size_t chain_len, string hass2find) {
    uint8_t *h_uint8 = (uint8_t *) malloc(33 * sizeof(uint8_t));
    char *pass = (char *) malloc(7 * sizeof(char));
    char *tmp = &first_pass_str[0];
    memcpy(pass, tmp, 7);
    pass[6] = '\0';
    blake256_hash(h_uint8, (uint8_t *) pass, 6);

    uint8_t *chass2find = string2uint8_t(hass2find, 64);
    size_t i = 1;
    while (memcmp(h_uint8, chass2find, 32)) {
        reduce(pass, h_uint8, i);
        blake256_hash(h_uint8, (uint8_t *) pass, 6);
        if (i >= CHAIN_LEN) {
            return ;
        }
        i++;
    }
    cout << "\n\nEUREKA!!!!!\n" << hass2find << " : " << pass << "\n\n" ;
    free(pass);
};

char* search_RT(string hstr64, unordered_map<string, string> chain_dict, size_t chain_len) {
    string h64 = hstr64;
    int i = chain_len-1;
    char *pass = (char*) malloc(7 * sizeof(char));
    while (i >= 0) {
        string first_pass = chain_dict[h64];
        h64 = hstr64;
        
        if (first_pass != "") {
            search_chain(first_pass, chain_len, hstr64);
        }
        for (int j = i ; j < chain_len ; j++) {

            uint8_t *temp_hash32 = string2uint8_t(h64, 64);
            reduce(pass, temp_hash32, j);
            blake256_hash(temp_hash32, (uint8_t *) pass, 6);
            h64 = uint8_t2string(temp_hash32, 64);
            free(temp_hash32);

        }
        i--;
    }
    free(pass);
    return NULL;
}

int main(int argc, char **argv) {
    size_t table_id;
    size_t num_of_tables = NUM_OF_TABLES;
    size_t chain_len = CHAIN_LEN;
    
    size_t start_table = 0;
    if (argc > 1) {
        start_table = atoi(argv[1]);
    } 

    unordered_map<string, string> chain_dict;   
    for (table_id = start_table ; table_id < start_table+num_of_tables ; table_id++) {
        char table_name[15];
        sprintf(table_name, "rainbow_%zu.csv", table_id);

        ifstream file(table_name);
        string first_pass, last_h;
        while (!file.eof()) {
            file >> first_pass;
            file >> last_h;
            chain_dict.insert(make_pair(last_h, first_pass));
        }

        cout << "Finish inserting from " << table_name << endl;
    }
    cout << "Inserting to unordered_map finished" << endl;

    string h_str;
    cout << "Enter a hash to search:";
    cin >> h_str;
    while (1) {
        char* passwd = search_RT(h_str, chain_dict, chain_len);
        if (passwd == NULL) {
            cout << "Unable to find the password for " << h_str << "\n";
        } else {
            cout << "Found password for " << h_str << " : " << passwd << "\n";
        }
        cout << "\nEnter another hash to search:";
        cin >> h_str;
    }

    return EXIT_SUCCESS;
}