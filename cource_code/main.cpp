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

sf::RenderWindow window(sf::VideoMode(1200, 720), "downloader");
sf::Event event;
enum modes{urltyping, displaying, passwording, answering, writting, ftpconnect};
enum crypt{de, en, en_fromfile};
enum answers{t, n, wait};
modes mode=displaying;
crypt crypting;
answers answer=wait;
string url, uri, key, filename, filebody, buffer, towrite;
bool mouseButton;
float spriteScale=1;
sf::Vector2f lastpos;
sf::Texture texture;
sf::Sprite sprite;
sf::Ftp ftp;
sf::Ftp::Response ftpresponse;

int main()
{
    system("color 0a");
    system("title displaying");
    sprite.setPosition(0,0);
    while(window.isOpen()){
        while(window.pollEvent(event)){
            if(event.type==sf::Event::Closed){
                window.close();
            }
            if(event.type==sf::Event::TextEntered){
                if(event.text.unicode==27){
                    mode=displaying;
                    system("title displaying");
                    url=uri=key=filename=filebody=buffer="";
                }
                if((event.text.unicode==116)||(event.text.unicode==49)||(event.text.unicode==121)||(event.text.unicode==84)||(event.text.unicode==89)||(event.text.unicode==13)||(event.text.unicode==111)||(event.text.unicode==79)){
                    answer=t;
                }else if((event.text.unicode==110)||(event.text.unicode==78)||(event.text.unicode==48)||(event.text.unicode==32)){
                    answer=n;
                }else answer=wait;

                if(mode==urltyping){
                    if(event.text.unicode==13){
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
                                while(1){
                                    cout<<"\ntype name \\/\n";
                                    mode=passwording;
                                    system("title passwording");
                                    filebody=response.getBody();
                                    if(uri.length()>4)uri.erase(0, uri.length()-4);
                                    if(uri==".jpg"){
                                        texture.loadFromMemory(&filebody[0], filebody.length());//http://www.programmingsimplified.com/images/c/delete-file-c.png
                                        sprite.setTexture(texture);
                                        break;
                                    }else
                                    if(uri==".png"){
                                        texture.loadFromMemory(&filebody[0], filebody.length());
                                        sprite.setTexture(texture);
                                        break;
                                    }else{
                                        mode=writting;
                                        system("title writting");
                                        towrite=filebody;
                                    }
                                break;
                                }
                            }else{
                                cout<<response.getStatus();
                                cout<<response.getBody();
                            }
                            if(mode!=passwording){
                                mode=displaying;
                                system("\ntitle displaying");
                            }
                        }
                        uri=url="";
                    }else
                    if(event.text.unicode==8){
                        if(url.length()){
                            url.erase(url.length()-1);
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
                            url+=(char*)(GetClipboardData(CF_TEXT));
                            cout<<(char*)(GetClipboardData(CF_TEXT));
                            CloseClipboard();
                        }
                    }else{
                        url+=char(event.text.unicode);
                        cout<<char(event.text.unicode);
                    }
                }else
                if(mode==passwording){
                    if(event.text.unicode==13){
                        if(filename.length()){
                            if(crypting==en){
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
                                save(filename, filebody);
                                filebody=key=filename="";
                                mode=displaying;
                                system("title displaying");
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

                                    sprite.setTexture(texture);

                                    if(filename.rfind('\\')==4294967295) filename="decrypted_"+filename;
                                    else filename.insert(filename.rfind('\\')+1, "decrypted_");
                                    if(texture.loadFromMemory(&buffer[0], buffer.length())){
                                        sprite.setTexture(texture);
                                        cout<<"\nsave file?";
                                        mode=answering;
                                    }else{
                                        mode=writting;
                                        system("title writting");
                                        towrite=buffer;
                                    }
                                    filebody=key=buffer=filename="";
                                    system("title displaying");
                                    mode=displaying;

                                }else{
                                    cout<<"\nfile loading error";
                                    filebody=key=buffer=filename="";
                                    system("title displaying");
                                    mode=displaying;
                                }
                                plik.close();
                            }else{//crypting==en_fromfile
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
                                    filebody=key=buffer=filename="";
                                    mode=displaying;
                                    system("title displaying");
                                }else{
                                    cout<<"\nfile loading error";
                                    filebody=key=buffer=filename="";
                                    system("title displaying");
                                    mode=displaying;
                                }
                            }
                        }else{
                            filename=key;
                            key="";
                            cout<<"\ntype password \\/\n";
                        }
                    }else
                    if(event.text.unicode==8){
                        if(key.length()){
                            key.erase(key.length()-1);system(("title "+to_string(key.length())).c_str());
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
                            key+=(char*)(GetClipboardData(CF_TEXT));
                            cout<<(char*)(GetClipboardData(CF_TEXT));
                            CloseClipboard();
                        }
                    }else{
                        key+=char(event.text.unicode);
                        cout<<char(event.text.unicode);
                    }
                }else
                if(mode==writting){
                    if(towrite.length()>=160){
                        cout<<towrite.substr(0, 160);
                        towrite.erase(0, 160);
                    }else{
                        if(towrite.length()){
                            cout<<towrite;
                            towrite="";
                        }
                        cout<<"\ndestroy file?";
                        mode=answering;
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
                if(event.text.unicode==100){
                    mode=passwording;
                    crypting=de;
                    cout<<"\ntype name\\/\n";
                    system("title passwording");
                }else
                if(event.text.unicode==101){
                    mode=passwording;
                    crypting=en_fromfile;
                    cout<<"\ntype name\\/\n";
                    system("title passwording");
                }else
                if(event.text.unicode==104){
                    cout<<"\n";
                    mode=urltyping;
                    crypting=en;
                    system("title urltyping");
                }else
                if(event.text.unicode==99){
                    system("cls");
                }else
                if((event.text.unicode==114)||(event.text.unicode==82)){
                    spriteScale=1;
                    sprite.setPosition(0,0);
                    sprite.setScale(1,1);
                }
            }
            if(event.type==sf::Event::MouseButtonPressed){
                mouseButton=1;
            }
            if(event.type==sf::Event::MouseButtonReleased){
                mouseButton=0;
            }
            if(event.type==sf::Event::MouseMoved){
                if(mouseButton) sprite.setPosition(sprite.getPosition()-lastpos+sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
                lastpos.x=event.mouseMove.x;
                lastpos.y=event.mouseMove.y;
            }
            if(event.type==sf::Event::MouseWheelMoved){
                float delta=(float(event.mouseWheel.delta+1)*0.422222222222)+0.666666666666;
                spriteScale*=delta;
                sprite.setScale(spriteScale, spriteScale);
                sprite.setPosition(sprite.getPosition().x-((event.mouseWheel.x-sprite.getPosition().x)*delta-(event.mouseWheel.x-sprite.getPosition().x)), sprite.getPosition().y-((event.mouseWheel.y-sprite.getPosition().y)*delta-(event.mouseWheel.y-sprite.getPosition().y)));
            }
            if(event.type==sf::Event::Resized) window.setSize(sf::Vector2u(1200, 720));
        }
    window.clear(sf::Color(50, 50, 50));
    window.draw(sprite);
    window.display();
    }
    return 0;
}
