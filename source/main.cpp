#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cstdlib>
#include <windows.h>
#include <fstream>
#include <curl/curl.h>


//I know, that`s spaghetti


using namespace std;

void getCursor(int &x, int&y) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        x = csbi.dwCursorPosition.X;
        y = csbi.dwCursorPosition.Y;
    }
}

void save(string track, string &input){
    if((!input.length())||(!track.length()))return;
    fstream plik;
    plik.open(track, ios::out | ios::binary);
    plik<<input;
    plik.close();
}

void destruct(string track, string &input){cout<<"\b\b\b";
    if((!input.length())||(!track.length()))return;
    fstream plik;
    plik.open(track, ios::out | ios::binary);
    string negative;
    int seci=input.length();
    for(int i=0; i<seci; i++){
        negative+=(~input[i]);
    }
    plik<<negative;
    plik.close();
    plik.clear();
    for(int i=0; i<32; i++){
        plik.open(track, ios::out | ios::binary);
        negative="";
        for(int j=0; j<seci; j++){
            negative+=rand()%256;
        }
        plik<<negative;
        plik.close();
        plik.clear();
    }
    remove(track.c_str());
}

bool copyToClipboard(string &input){
    HGLOBAL hglbCopy;
    LPTSTR  lptstrCopy;

    if(!OpenClipboard(0))
        return 0;
    EmptyClipboard();
    if(!input.length()){
        CloseClipboard();
        return 0;
    }

    hglbCopy=GlobalAlloc(GMEM_MOVEABLE, input.length()+1);
    if(!hglbCopy){
        CloseClipboard();
        return 0;
    }

    memcpy(GlobalLock(hglbCopy), &input[0], input.length()+1);
    GlobalUnlock(hglbCopy);
    SetClipboardData(CF_TEXT, hglbCopy);
    CloseClipboard();
    return 1;
}

sf::RenderWindow window(sf::VideoMode(1200, 720), "downloader");
sf::Event event;
enum modes{urltyping, displaying, passwording, answering, writting, ftpconnect};
enum crypt{de, en, en_fromfile, text};
enum answers{t, n, wait};
modes mode=displaying;
crypt crypting;
answers answer=wait;
string url, uri, key, filename, filebody, buffer, header;
bool mouseButton, mask, TnI=0/*0=text, 1=image*/;
float spriteScale=1;
sf::Vector2f lastpos, utextdelta(0,40);
sf::Texture texture, maskTexture, downloadingTexture, downloadTexture, deleteTexture, decryptTexture, decryptingTexture, encryptTexture, encryptingTexture, resetTexture, textpointerTexture;
sf::Sprite sprite, maskSprite, downloadingSprite, downloadSprite, deleteSprite, decryptSprite, decryptingSprite, encryptSprite, encryptingSprite, resetSprite, textpointerSprite;
sf::Ftp ftp;
sf::Ftp::Response ftpresponse;
sf::Font mainfont, full_ascii;
sf::Text infotext, unknowntext;
sf::RectangleShape greybar(sf::Vector2f(1200, 40));
int charsize=14, cursorpos=0, timer=0;

void resetbuffers(){
    mode=displaying;
    system("title displaying");
    url=uri=key=filename=buffer="";
    infotext.setString(header);
}

void centerText(){
    if(textpointerSprite.getPosition().x>1198){
        utextdelta.x-=textpointerSprite.getPosition().x-1198;
        textpointerSprite.setPosition(1198, textpointerSprite.getPosition().y);
        unknowntext.setPosition(utextdelta);
    }else
    if(textpointerSprite.getPosition().x<0){
        utextdelta.x-=textpointerSprite.getPosition().x;
        unknowntext.setPosition(utextdelta);
        textpointerSprite.setPosition(0, textpointerSprite.getPosition().y);
    }
    if(textpointerSprite.getPosition().y<40){
        utextdelta.y-=textpointerSprite.getPosition().y-40;
        unknowntext.setPosition(utextdelta);
        textpointerSprite.setPosition(textpointerSprite.getPosition().x, 40);
    }else
    if(textpointerSprite.getPosition().y>705){
        utextdelta.y-=textpointerSprite.getPosition().y-705;
        unknowntext.setPosition(utextdelta);
        textpointerSprite.setPosition(textpointerSprite.getPosition().x, 705);
    }
}

int main()
{
    {   system("color 0a");
        system("title displaying");
        window.setFramerateLimit(30);
        sprite.setPosition(0,40);
        maskTexture.loadFromFile("img/mask.png");
        maskSprite.setTexture(maskTexture);
        downloadTexture.loadFromFile("img/checkbox_download.bmp");
        downloadSprite.setTexture(downloadTexture);
        downloadSprite.setPosition(0,0);
        downloadingTexture.loadFromFile("img/checkbox_downloading.bmp");
        downloadingSprite.setTexture(downloadingTexture);
        downloadingSprite.setPosition(0,0);
        deleteTexture.loadFromFile("img/checkbox_delete.bmp");
        deleteSprite.setTexture(deleteTexture);
        deleteSprite.setPosition(40,0);
        decryptTexture.loadFromFile("img/checkbox_decrypt.bmp");
        decryptSprite.setTexture(decryptTexture);
        decryptSprite.setPosition(80,0);
        encryptTexture.loadFromFile("img/checkbox_encrypt.bmp");
        encryptSprite.setTexture(encryptTexture);
        encryptSprite.setPosition(120,0);
        decryptingTexture.loadFromFile("img/checkbox_decrypting.bmp");
        decryptingSprite.setTexture(decryptingTexture);
        decryptingSprite.setPosition(80,0);
        encryptingTexture.loadFromFile("img/checkbox_encrypting.bmp");
        encryptingSprite.setTexture(encryptingTexture);
        encryptingSprite.setPosition(120,0);
        resetTexture.loadFromFile("img/checkbox_reset.bmp");
        resetSprite.setTexture(resetTexture);
        resetSprite.setPosition(160,0);
        textpointerTexture.loadFromFile("img/pointer.bmp");
        textpointerSprite.setTexture(textpointerTexture);
        textpointerSprite.setPosition(utextdelta);
        greybar.setPosition(0,0);
        greybar.setFillColor(sf::Color(50,50,50));
        mainfont.loadFromFile("font.ttf");
        full_ascii.loadFromFile("SourceCodePro-Regular.ttf");
        infotext.setFont(mainfont);
        infotext.setColor(sf::Color(0,255,0));
        infotext.setCharacterSize(14);
        infotext.setPosition(200, 0);
        unknowntext.setFont(full_ascii);
        unknowntext.setColor(sf::Color(0,255,0));
        unknowntext.setCharacterSize(14);
        unknowntext.setPosition(utextdelta);
        header="downloader v1.2 by Aleksander Czajka\npress k for key biddings\n\n";
        infotext.setString(header);
    }
    cout<<header;
    while(window.isOpen()){
        while(window.pollEvent(event)){
            if(event.type==sf::Event::Closed){
                window.close();
            }
            if(event.type==sf::Event::TextEntered){
                if(event.text.unicode==27){
                    resetbuffers();
                    TnI=0;
                }
                if((event.text.unicode==116)||(event.text.unicode==49)||(event.text.unicode==121)||(event.text.unicode==84)||(event.text.unicode==89)||(event.text.unicode==13)||(event.text.unicode==111)||(event.text.unicode==79)){
                    answer=t;
                }else if((event.text.unicode==110)||(event.text.unicode==78)||(event.text.unicode==48)||(event.text.unicode==32)){
                    answer=n;
                }else answer=wait;

                if(mode==urltyping){
                    if(event.text.unicode==13){cout<<"\n";
                        if(!url.find("ftpconnect")){
                            mode=ftpconnect;
                            system("title ftp");
                            ftpresponse=ftp.connect(url);
                            if(ftpresponse.isOk()){
                                cout<<"\nconnected";
                            }else{
                                cout<<"\nconnection error, try again\n";
                            }
                        }else{
                            if((url.find("http://"))&&(url.find("https://"))){
                                if(!url.find("www."))url="http://"+url;
                                else url="http://www."+url;
                            }
                            if(url.find('/', 9)<100000){
                                uri=url;
                                uri.erase(0, url.find('/', 9));
                                url.erase(url.find('/', 9), url.length());
                            }
                            sf::Http http;//---------------------------------------------------http(s) request------------------------------------------------
                            http.setHost(url);
                            sf::Http::Request request(uri);
                            sf::Http::Response response=http.sendRequest(request);
                            if(response.getStatus()==sf::Http::Response::Ok){
                                filebody=response.getBody();
                                TnI=0;
                                if(texture.loadFromMemory(&filebody[0], filebody.length())){//http://www.programmingsimplified.com/images/c/delete-file-c.png
                                    sprite.setTexture(texture);
                                    cout<<"\ntype name \\/\n";
                                    infotext.setString("name=");
                                    mode=passwording;
                                    system("title passwording");
                                    break;
                                }else{
                                    TnI=1;
                                    mode=writting;
                                    system("title writting");
                                    infotext.setString("Unknown format.");
                                }
                                unknowntext.setString(filebody);
                                cursorpos=0;
                            }else{
                                cout<<response.getStatus();
                                cout<<response.getBody();
                                mode=displaying;
                                system("title displaying");
                                resetbuffers();
                                infotext.setString(header);
                            }
                        }
                        uri=url="";
                    }else
                    if(event.text.unicode==8){
                        if(url.length()){
                            url.erase(url.length()-1);
                            buffer=infotext.getString();
                            infotext.setString(buffer.erase(buffer.length()-1));
                            if(!((key.length()+1)%80)){
                                int x, y;
                                getCursor(x, y);
                                COORD coord={79, y-1};
                                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                                cout<<" ";
                                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                            }else
                                cout<<"\b \b";
                        }
                    }else
                    if(event.text.unicode==9){
                        if(OpenClipboard(0)){
                            buffer=infotext.getString();
                            buffer+=(char*)(GetClipboardData(CF_TEXT));
                            infotext.setString(buffer);
                            url+=(char*)(GetClipboardData(CF_TEXT));
                            cout<<(char*)(GetClipboardData(CF_TEXT));
                            CloseClipboard();
                        }
                    }else
                    if(event.text.unicode==127){
                        while(url.length()){
                            url.erase(url.length()-1);
                            buffer=infotext.getString();
                            infotext.setString(buffer.erase(buffer.length()-1));
                            if(!((key.length()+1)%80)){
                                int x, y;
                                getCursor(x, y);
                                COORD coord={79, y-1};
                                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                                cout<<" ";
                                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                            }else
                                cout<<"\b \b";
                        }
                    }else{
                        url+=char(event.text.unicode);
                        cout<<char(event.text.unicode);
                        infotext.setString(infotext.getString()+char(event.text.unicode));
                    }
                }else
                if(mode==passwording){
                    if(event.text.unicode==13){
                        if(filename.length()){
                            if(crypting==en){
                                int seci=key.length();
                                if(seci){
                                    long long srand=0, srand2=0;
                                    for(int i=0; i<seci; i++){
                                        srand=srand<<6;
                                        srand+=key[i]%64;
                                    }
                                    for(int i=seci-1; i>=0; i--){
                                        srand2=srand2<<6;
                                        srand2+=key[i]%64;
                                    }
                                    //_______________________________________
                                    std::srand(srand);
                                    seci=filebody.length();
                                    for(int i=0; i<seci; i++){
                                        filebody[i]=filebody[i]^(rand()%256);
                                    }
                                    std::srand(srand2);
                                    for(int i=seci-1; i>=0; i--){
                                        filebody[i]=filebody[i]^(rand()%256);
                                    }//--------------------------------------
                                }
                                save(filename, filebody);
                                resetbuffers();
                                infotext.setString(header);
                            }else
                            if(crypting==de){
                                fstream plik;
                                plik.open(filename, std::ios::in | std::ios::binary);
                                if(plik.good()){
                                    buffer="";
                                    char ch;
                                    while(plik>>noskipws>>ch){
                                        buffer+=ch;
                                    }
                                    plik.close();
                                    int seci=key.length();
                                    if(seci){
                                        long long srand=0, srand2=0;
                                        for(int i=0; i<seci; i++){
                                            srand=srand<<6;
                                            srand+=key[i]%64;
                                        }
                                        for(int i=seci-1; i>=0; i--){
                                            srand2=srand2<<6;
                                            srand2+=key[i]%64;
                                        }
                                        //____________________________________
                                        std::srand(srand);
                                        seci=buffer.length();

                                        for(int i=0; i<seci; i++){
                                            buffer[i]=buffer[i]^(rand()%256);
                                        }
                                        std::srand(srand2);
                                        for(int i=seci-1; i>=0; i--){
                                            buffer[i]=buffer[i]^(rand()%256);
                                        }//-----------------------------------
                                    }
                                    if(texture.loadFromMemory(&buffer[0], buffer.length())){
                                        sprite.setTexture(texture);
                                        unknowntext.setString(buffer);
                                        filebody=buffer;
                                        resetbuffers();
                                        TnI=0;
                                        system("title displaying");
                                        mode=displaying;
                                    }else{
                                        unknowntext.setString(buffer);
                                        filebody=buffer;
                                        TnI=1;
                                        system("title writting");
                                        mode=writting;
                                        crypting=text;
                                    }
                                    cursorpos=0;
                                }else{
                                    cout<<"\nfile loading error";
                                    resetbuffers();
                                }
                                plik.close();
                                infotext.setString(header);
                            }else
                            if(crypting==en_fromfile){
                                fstream plik;
                                plik.open(filename, std::ios::in | std::ios::binary);
                                if(plik.good()){
                                    buffer="";
                                    char ch;
                                    while(plik>>noskipws>>ch){
                                        buffer+=ch;
                                    }
                                    plik.clear();
                                    plik.close();
                                    long long srand=0, srand2=0;
                                    int seci=key.length();
                                    for(int i=0; i<seci; i++){
                                        srand=srand<<6;
                                        srand+=key[i]%64;
                                    }
                                    for(int i=seci-1; i>=0; i--){
                                        srand2=srand2<<6;
                                        srand2+=key[i]%64;
                                    }
                                    std::srand(srand);
                                    //aliexpress.com______________________
                                    seci=buffer.length();
                                    for(int i=0; i<seci; i++){
                                        buffer[i]=buffer[i]^(rand()%256);
                                    }
                                    std::srand(srand2);
                                    for(int i=seci-1; i>=0; i--){
                                        buffer[i]=buffer[i]^(rand()%256);
                                    }//-----------------------------------

                                    save(filename, buffer);
                                    cout<<"\noverwrited\n";
                                    resetbuffers();
                                }else{
                                    cout<<"\nfile loading error";
                                    resetbuffers();
                                }
                            }
                            else{//crypting==text
                                int seci=key.length();
                                if(seci){
                                    long long srand=0, srand2=0;
                                    for(int i=0; i<seci; i++){
                                        srand=srand<<6;
                                        srand+=key[i]%64;
                                    }
                                    for(int i=seci-1; i>=0; i--){
                                        srand2=srand2<<6;
                                        srand2+=key[i]%64;
                                    }
                                    //_______________________________________
                                    std::srand(srand);
                                    seci=filename.length();
                                    for(int i=0; i<seci; i++){
                                        filename[i]=filename[i]^(rand()%256);
                                    }
                                    std::srand(srand2);
                                    for(int i=seci-1; i>=0; i--){
                                        filename[i]=filename[i]^(rand()%256);
                                    }//--------------------------------------
                                }
                                if(texture.loadFromMemory(&filename[0], filename.length())){
                                    sprite.setTexture(texture);
                                    unknowntext.setString(filename);
                                    filebody=filename;
                                    resetbuffers();
                                    TnI=0;
                                    system("title displaying");
                                    mode=displaying;
                                }else{
                                    unknowntext.setString(filename);
                                    filebody=filename;
                                    TnI=1;
                                    system("title writting");
                                    mode=writting;
                                }
                            }
                            infotext.setString(header);
                        }else{
                            filename=key;
                            key="";
                            cout<<"\ntype password \\/\n";
                            infotext.setString("password=");
                        }
                    }else
                    if(event.text.unicode==8){
                        if(key.length()){
                            key.erase(key.length()-1);          //system(("title "+to_string(key.length())).c_str());
                            buffer=infotext.getString();
                            infotext.setString(buffer.erase(buffer.length()-1));
                            if(!((key.length()+1)%80)){
                                int x, y;
                                getCursor(x, y);
                                COORD coord={79, y-1};
                                if(key.length())SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                                cout<<" ";
                                if(key.length()) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                            }else
                                cout<<"\b \b";
                        }
                    }else
                    if(event.text.unicode==9){
                        if(OpenClipboard(0)){
                            buffer=infotext.getString();
                            buffer+=(char*)(GetClipboardData(CF_TEXT));
                            infotext.setString(buffer);
                            key+=(char*)(GetClipboardData(CF_TEXT));
                            cout<<(char*)(GetClipboardData(CF_TEXT));
                            CloseClipboard();
                        }
                    }else
                    if(event.text.unicode==127){
                        while(key.length()){
                            key.erase(key.length()-1);
                            buffer=infotext.getString();
                            infotext.setString(buffer.erase(buffer.length()-1));
                            if(!((key.length()+1)%80)){
                                int x, y;
                                getCursor(x, y);
                                COORD coord={79, y-1};
                                if(key.length())SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                                cout<<" ";
                                if(key.length()) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord );
                            }else
                                cout<<"\b \b";
                        }
                    }else{
                        infotext.setString(infotext.getString()+char(event.text.unicode));
                        key+=char(event.text.unicode);
                        cout<<char(event.text.unicode);
                    }
                }else
                if(mode==writting){
                    if(event.text.unicode==8){
                        if((cursorpos)&&(filebody.length())){
                            unknowntext.setString(filebody.erase(cursorpos-1, 1));
                            cursorpos--;
                            textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        }
                    }else
                    if(event.text.unicode==127){
                        filebody="";
                        unknowntext.setString("");
                        cursorpos=0;
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                    }else
                    if((event.text.unicode==17)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        mode=passwording;
                        filename=filebody;
                        cout<<"\ntype password \\/\n";
                        infotext.setString("password=");
                        crypting=text;
                    }else
                    if((event.text.unicode==22)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        if(OpenClipboard(0)){
                            int i=filebody.length();
                            filebody.insert(cursorpos, (char*)(GetClipboardData(CF_TEXT)));
                            unknowntext.setString(filebody);
                            i=filebody.length()-i;
                            cursorpos+=i;
                            textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                            centerText();
                            CloseClipboard();
                        }
                    }else
                    if((event.text.unicode==3)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        copyToClipboard(filebody);
                    }
                    else{
                        unsigned char etu=event.text.unicode;
                        if(etu==13)etu=10;
                        unknowntext.setString(filebody.insert(cursorpos, 1, etu));
                        cursorpos++;
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                    }
                }else
                if(mode==answering){
                    if(answer!=wait){
                        if(answer==t){
                            cout<<"\nsaving...";
                            save(filename, buffer);
                        }
                        cout<<"\nclearing buffers\n";
                        filename=key=buffer="";
                        mode=displaying;
                        system("title displaying");
                    }
                }else
                if(mode==ftpconnect){

                }else
                if(event.text.unicode==99){
                    system("cls");
                    cout<<header;
                }else
                if(event.text.unicode==100){
                    mode=passwording;
                    crypting=de;
                    cout<<"\ndecrypting:\ntype name\\/\n";
                    infotext.setString("name=");
                    system("title passwording");
                }else
                if(event.text.unicode==101){
                    mode=passwording;
                    crypting=en_fromfile;
                    cout<<"\nencrypting from file:\ntype name\\/\n";
                    infotext.setString("name=");
                    system("title passwording");
                }else
                if(event.text.unicode==104){
                    cout<<"\ndownloading:\ntype URL\\/\n";
                    infotext.setString("URL=");
                    mode=urltyping;
                    crypting=en;
                    system("title urltyping");
                }else
                if(event.text.unicode==107){
                    cout<<"console`s header shows current mode.\nh = http request (unfortunately doesn`t support https)\nd = decrypt from disc\ne = encrypt from disc and overwrite\nm = turns mask on/off\nc = clear screen\nr = reset position and scale of image\nw = writting mode\n   ctrl+q = en/de crypt text\n   ctrl+backspace = delete all text\n   ctrl+c = copy all text\n   ctrl+v = paste\nesc = back to display mode and clear all buffers\n\nencryption and decryption are the same operations because of algoritm used here.\n";
                }else
                if(event.text.unicode==109){
                    mask=!mask;
                }else
                if(event.text.unicode==119){
                    mode=writting;
                    TnI=1;
                    crypting=text;
                }else
                if((event.text.unicode==114)||(event.text.unicode==82)){
                    spriteScale=1;
                    sprite.setPosition(0,40);
                    sprite.setScale(1,1);
                }
            }else
            if(event.type==sf::Event::KeyPressed){
                if(mode==writting){
                    if((event.key.code==sf::Keyboard::Left)&&(cursorpos>0)){
                        cursorpos--;
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        centerText();
                        timer=0;
                    }else
                    if((event.key.code==sf::Keyboard::Right)&&(cursorpos<filebody.length())){
                        cursorpos++;
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        centerText();
                        timer=0;
                    }else
                    if(event.key.code==sf::Keyboard::Down){
                        unsigned int findings[2]={0,0};
                        if(cursorpos)findings[0]=cursorpos-filebody.rfind('\n', cursorpos-1)-1;
                        findings[1]=filebody.find('\n', cursorpos);
                        if(findings[1]!=string::npos){
                            cursorpos=findings[1]+1;
                            findings[1]=filebody.find('\n', cursorpos);
                            if(findings[1]!=string::npos){
                                if(findings[1]-cursorpos>findings[0])
                                    cursorpos+=findings[0];
                                else
                                    cursorpos=findings[1];
                            }else
                            if(filebody.length()>cursorpos+findings[0])
                                cursorpos+=findings[0];
                            else
                                cursorpos=filebody.length();
                        }
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        timer=0;
                        centerText();
                    }else
                    if((event.key.code==sf::Keyboard::Up)&&(cursorpos)){
                        unsigned int findings[2]={0,0};
                        findings[1]=filebody.rfind('\n', cursorpos-1);
                        if(findings[1]==string::npos){
                            cursorpos=0;
                        }else{
                            findings[0]=cursorpos-findings[1]-1;
                            cursorpos=findings[1];
                            findings[1]=filebody.rfind('\n', cursorpos-1);
                            if(findings[1]==string::npos)
                            if(findings[1]+1+findings[0]<cursorpos) cursorpos=findings[1]+1+findings[0];
                            else cursorpos--;
                        }
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        timer=0;
                        centerText();
                    }
                    if(event.key.code==sf::Keyboard::Delete){
                        if(cursorpos<filebody.length()){
                            filebody.erase(cursorpos, 1);
                            unknowntext.setString(filebody);
                        }
                    }
                }
            }else
            if(event.type==sf::Event::MouseButtonPressed){
                mouseButton=1;
            }else
            if(event.type==sf::Event::MouseButtonReleased){
                mouseButton=0;
                if((event.mouseButton.x<downloadSprite.getPosition().x+downloadTexture.getSize().x)&&(event.mouseButton.y<downloadSprite.getPosition().y+downloadTexture.getSize().y)&&(event.mouseButton.x>=downloadSprite.getPosition().x)&&(event.mouseButton.y>=downloadSprite.getPosition().y)){
                    cout<<"\ndownloading:\ntype URL\\/\n";
                    resetbuffers();
                    infotext.setString("URL=");
                    mode=urltyping;
                    crypting=en;
                    system("title urltyping");
                }else
                if((event.mouseButton.x<deleteSprite.getPosition().x+deleteTexture.getSize().x)&&(event.mouseButton.y<deleteSprite.getPosition().y+deleteTexture.getSize().y)&&(event.mouseButton.x>=deleteSprite.getPosition().x)&&(event.mouseButton.y>=deleteSprite.getPosition().y)){
                    texture.create(1,1);
                    sprite.setTexture(texture);
                    cout<<"\nimage cleared\n";
                }else
                if((event.mouseButton.x<decryptSprite.getPosition().x+decryptTexture.getSize().x)&&(event.mouseButton.y<decryptSprite.getPosition().y+decryptTexture.getSize().y)&&(event.mouseButton.x>=decryptSprite.getPosition().x)&&(event.mouseButton.y>=decryptSprite.getPosition().y)){
                    resetbuffers();
                    mode=passwording;
                    crypting=de;
                    cout<<"\ndecrypting:\ntype name\\/\n";
                    infotext.setString("name=");
                    system("title passwording");
                }else
                if((event.mouseButton.x<resetSprite.getPosition().x+resetTexture.getSize().x)&&(event.mouseButton.y<resetSprite.getPosition().y+resetTexture.getSize().y)&&(event.mouseButton.x>=resetSprite.getPosition().x)&&(event.mouseButton.y>=resetSprite.getPosition().y)){
                    sprite.setPosition(0,40);
                    sprite.setScale(1,1);
                }else
                if((event.mouseButton.x<encryptSprite.getPosition().x+encryptTexture.getSize().x)&&(event.mouseButton.y<encryptSprite.getPosition().y+encryptTexture.getSize().y)&&(event.mouseButton.x>=encryptSprite.getPosition().x)&&(event.mouseButton.y>=encryptSprite.getPosition().y)){
                    resetbuffers();
                    mode=passwording;
                    crypting=en_fromfile;
                    cout<<"\nencrypting from file:\ntype name\\/\n";
                    infotext.setString("name=");
                    system("title passwording");
                }
            }else
            if(event.type==sf::Event::MouseMoved){
                if(mouseButton) sprite.setPosition(sprite.getPosition()-lastpos+sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
                lastpos.x=event.mouseMove.x;
                lastpos.y=event.mouseMove.y;
            }else
            if(event.type==sf::Event::MouseWheelMoved){
                if(!TnI){
                    float delta=(float(event.mouseWheel.delta+1)*0.422222222222)+0.666666666666;
                    spriteScale*=delta;
                    sprite.setScale(spriteScale, spriteScale);
                    sprite.setPosition(sprite.getPosition().x-((event.mouseWheel.x-sprite.getPosition().x)*delta-(event.mouseWheel.x-sprite.getPosition().x)), sprite.getPosition().y-((event.mouseWheel.y-sprite.getPosition().y)*delta-(event.mouseWheel.y-sprite.getPosition().y)));
                }
                else{
                    utextdelta.y+=event.mouseWheel.delta*20;
                    textpointerSprite.move(0,event.mouseWheel.delta*20);
                    unknowntext.setPosition(utextdelta);
                    timer=0;
                }
            }else
            if(event.type==sf::Event::Resized) window.setSize(sf::Vector2u(1200, 720));
        }

        timer++;
        if(timer>29)timer=0;

        window.clear(sf::Color(50, 50, 50));
        if(TnI){
            window.draw(unknowntext);
            if(timer<15)window.draw(textpointerSprite);
        }
        else window.draw(sprite);
        if(mask)
            for(int i=0; i<40/* 1200/30 */; i++){
                for(int j=1; j<36/* 720/20 */; j++){
                    maskSprite.setPosition(i*30, j*20);
                    window.draw(maskSprite);
                }
            }
        window.draw(greybar);
        window.draw(downloadSprite);
        window.draw(encryptSprite);
        window.draw(decryptSprite);
        if(mode!=displaying)
            if(crypting==en)              window.draw(downloadingSprite);
            else if(crypting==en_fromfile)window.draw(encryptingSprite);
            else if(crypting==de)         window.draw(decryptingSprite);
        window.draw(deleteSprite);
        window.draw(resetSprite);
        window.draw(infotext);
        window.display();
    }
    return 0;
}
