#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "cJSON.h"

// Kırmızı renk kodu tanımı
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"


// DNS'nin geçerli olup olmadığını ve maksimum boyutu aşıp aşmadığını kontrol eden fonksiyon
bool isValidDNS(const char *dns) {
    int num, dots = 0;
    char c;

    // Her bir bölümü kontrol etmek için döngü
    while (*dns) {
        // Rakam mı kontrol et
        if (isdigit(*dns)) {
            num = 0;
            // Rakamı oku ve num değişkenine kaydet
            while (isdigit(*dns)) {
                num = num * 10 + (*dns - '0');
                // Maksimum değeri aşma kontrolü
                if (num > 255) {
                    return false;
                }
                dns++;
            }
            // Rakamdan sonra . mı var kontrol et
            if (*dns == '.') {
                dns++;
                dots++;
                if (dots > 3) {
                    return false;
                }
            } else if (*dns && *dns != '.') {
                return false;
            }
        } else {
            return false;
        }
    }
    return (dots == 3);
}


void ping_dns(const char *dns) {
    char command[256];
    snprintf(command, sizeof(command), "ping -c 3 -q %s", dns);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen hatası");
        return;
    }

    char result[1024];
    bool success = false;
    while (fgets(result, sizeof(result), fp) != NULL) {
        if (strstr(result, "rtt min/avg/max/mdev") != NULL) {
            double min, avg, max, mdev;
            sscanf(result, "rtt min/avg/max/mdev = %lf/%lf/%lf/%lf ms", &min, &avg, &max, &mdev);

            const char *color;
            const char *status;
            if (avg <= 60) {
                color = "\033[0;32m";  // Yeşil
                status = "(Mükemmel)";
            } else if (avg <= 110) {
                color = "\033[0;33m";  // Sarı
                status = "(Orta)";
            } else {
                color = "\033[0;31m";  // Kırmızı
                status = "(Berbat)";
            }

            printf("%s%s -- %.2fms %s\033[0m\n", color, dns, avg, status);
            success = true;
        }
    }

    if (!success) {
        printf("\033[0;31mDNS adresine ping atılamadı. Bu DNS adresi bulunamayabilir veya kapalı olabilir.\033[0m\n");
    }

    pclose(fp);
}

void ping_default_dns(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Dosya açılamadı");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(length + 1);
    if (data) {
        fread(data, 1, length, file);
        data[length] = '\0';
    }
    fclose(file);

    cJSON *json = cJSON_Parse(data);
    if (!json) {
        fprintf(stderr, "JSON verisi işlenemedi: %s\n", cJSON_GetErrorPtr());
        free(data);
        return;
    }

    cJSON *groups = cJSON_GetObjectItem(json, "groups");
    if (!cJSON_IsArray(groups)) {
        fprintf(stderr, "JSON dosyası uygun formatta değil.\n");
        cJSON_Delete(json);
        free(data);
        return;
    }

    int size = cJSON_GetArraySize(groups);
    for (int i = 0; i < size; ++i) {
        cJSON *group = cJSON_GetArrayItem(groups, i);
        cJSON *groupName = cJSON_GetObjectItem(group, "name");
        cJSON *dnsList = cJSON_GetObjectItem(group, "dns");

        if (cJSON_IsString(groupName) && cJSON_IsArray(dnsList)) {
            printf("%s:\n", groupName->valuestring);
            int dnsCount = cJSON_GetArraySize(dnsList);
            for (int j = 0; j < dnsCount; ++j) {
                cJSON *dns = cJSON_GetArrayItem(dnsList, j);
                if (cJSON_IsString(dns)) {
                    ping_dns(dns->valuestring);
                }
            }
            printf("\n");
        }
    }

    cJSON_Delete(json);
    free(data);
}

void ping_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Dosya açılamadı");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(length + 1);
    if (data) {
        fread(data, 1, length, file);
        data[length] = '\0';
    }
    fclose(file);

    cJSON *json = cJSON_Parse(data);
    if (!json) {
        fprintf(stderr, "JSON verisi işlenemedi: %s\n", cJSON_GetErrorPtr());
        free(data);
        return;
    }

    cJSON *groups = cJSON_GetObjectItem(json, "groups");
    if (!cJSON_IsArray(groups)) {
        fprintf(stderr, "JSON dosyası uygun formatta değil.\n");
        cJSON_Delete(json);
        free(data);
        return;
    }

    int size = cJSON_GetArraySize(groups);
    for (int i = 0; i < size; ++i) {
        cJSON *group = cJSON_GetArrayItem(groups, i);
        cJSON *groupName = cJSON_GetObjectItem(group, "name");
        cJSON *dnsList = cJSON_GetObjectItem(group, "dns");

        if (cJSON_IsString(groupName) && cJSON_IsArray(dnsList)) {
            printf("%s DNS:\n", groupName->valuestring);
            int dnsCount = cJSON_GetArraySize(dnsList);
            for (int j = 0; j < dnsCount; ++j) {
                cJSON *dns = cJSON_GetArrayItem(dnsList, j);
                if (cJSON_IsString(dns)) {
                    ping_dns(dns->valuestring);
                }
            }
            printf("\n");
        }
    }

    cJSON_Delete(json);
    free(data);
}

// Help Yardım Mesajı Funksiyonu
int help() {
    printf("\n");
    printf("UBeDNS parametreleri:\n");
    //df parametresi
    printf("--df \t Belirlediğiniz DNS listesi dosyasındaki DNS adreslerine ping atar.");
    printf("\nKullanımı: ubedns --df [Dosya]\n\n");
    //c parametresi
    printf("--cd ya da -c \t Belirlediğiniz tek bir DNS adresine ping atar.");
    printf("\nKullanımı: ubedns --cd [DNS adresi] ya da ubedns -c [DNS adresi]\n\n");
    //help parametresi
    printf("--help ya da -h \t Bu mesajı görmenizi sağlar.");
    printf("\nKullanımı: ubedns --help ya da ubedns -h\n\n");
    // start parametresi
    printf("--start ya da -s \t Varsayılan DNS listesindeki DNS adreslerine ping atar.");
    printf("\nKullanımı: ubedns --start ya da ubedns -s\n\n");
    return 0;
}

// Main funksiyonu
int main(int argc, char *argv[]) {
    // default dns jsonu bul
    const char *home_dir = getenv("HOME");
    char default_filename[256];
    snprintf(default_filename, sizeof(default_filename), "%s/.config/ubedns/default_dns.json", home_dir);

    // her seyi temizle
    system("clear");

    // kodlar
    //const char *default_filename = "default_dns.json";
    // --df , df parametresi
    if (argc == 3 && strcmp(argv[1], "--df") == 0) {
        ping_from_file(argv[2]);
        return 0;
    }

    // --help , -h , help parametresi
    if (argc == 2 && strcmp(argv[1], "--help") == 0 || argc == 2 && strcmp(argv[1], "-h") == 0) {
        help();
        return 0;
    }

    // --cd , -c , custom dns parametresı
    if (argc == 3 && (strcmp(argv[1], "--cd") == 0 || strcmp(argv[1], "-c") == 0)) {
        if(isValidDNS(argv[2])) {
            printf("\nPinging %s...\n", argv[2]);
            ping_dns(argv[2]);
            return 0;
        } else {
            printf(ANSI_COLOR_RED "Geçersiz DNS adresi. Lütfen geçerli bir DNS adresi girin.\n" ANSI_COLOR_RESET);
            return 0;
        }
    }

    if (argc == 2 && strcmp(argv[1], "--start") == 0 || argc == 2 && strcmp(argv[1], "-s") == 0) {
        ping_default_dns("default_dns.json");
        return 0;
    }

    char input[256];
    while (true) {
        printf("\nS - Varsayılan DNS'lere ping at\n");
        printf("C [Custom DNS] - Belirtilen DNS'e ping at\n");
        printf("H - Daha fazla bilgi (yardım)\n");
        printf("Q - Programdan çık\n");
        printf("Seçiminizi yapın: ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        if (input[0] == 'Q' || input[0] == 'q') {
            break;
        } else if (input[0] == 'S' || input[0] == 's') {
            printf("\n");
            ping_default_dns(default_filename);
        } else if (input[0] == 'C' || input[0] == 'c') {
            char dns[256];
            if (sscanf(input, "C %255s", dns) == 1 || sscanf(input, "c %255s", dns) == 1) {
                printf("\nPinging %s...\n", dns);
                fflush(stdout);
                ping_dns(dns);
            } else {
                printf("Geçersiz DNS formatı. Lütfen tekrar deneyin.\n");
            }
        } else if (input[0] == 'H' || input[0] == 'h'){
            help();
        } else {
            printf("Geçersiz seçenek. Lütfen tekrar deneyin.\n");
        }
        printf("\n"); 
    }

    return 0;
}
