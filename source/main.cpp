#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cstdlib>
#include <windows.h>
#include <fstream>
#include <curl/curl.h>
#include <vector>
#include <list>


//I know, that`s spaghetti


using namespace std;

void getCursor(int &x, int&y) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        x = csbi.dwCursorPosition.X;
        y = csbi.dwCursorPosition.Y;
    }
}

bool save(string track, string &input){
    if((!input.length())||(!track.length()))return 0;
    fstream plik;
    plik.open(track, ios::out | ios::binary);
    if(!plik.good()){
        vector<string> buffers;
        bool nofilename=0;
        size_t lastrslash=track.rfind('/'), lastlslash=track.rfind('\\');
        if((lastrslash==track.size()-1)||(lastlslash==track.size()-1)){
            cout<<"no filename, creating folder tree\n";
            nofilename=1;
        }
        while(1){
            buffers.push_back("");
            if(buffers.size()>1){
                buffers[buffers.size()-1]=buffers[buffers.size()-2];
                lastrslash=buffers[buffers.size()-1].rfind('/');
                lastlslash=buffers[buffers.size()-1].rfind('\\');
            }else
                buffers[0]=track;
            if(lastrslash!=string::npos){
                if(lastlslash!=string::npos){
                    if(lastlslash>lastrslash){
                        if(!lastlslash) break;
                        if(lastlslash+1==buffers[buffers.size()-1].size()){
                            cout<<"empty folder name\n";
                            return 0;
                        }
                        buffers[buffers.size()-1].erase(buffers[buffers.size()-1].begin()+lastlslash, buffers[buffers.size()-1].end());
                    }else{
                        if(!lastrslash) break;
                        if(lastrslash+1==buffers[buffers.size()-1].size()){
                            cout<<"empty folder name\n";
                            return 0;
                        }
                        buffers[buffers.size()-1].erase(buffers[buffers.size()-1].begin()+lastrslash, buffers[buffers.size()-1].end());
                    }
                }else{
                    if(!lastrslash) break;
                    if(lastrslash+1==buffers[buffers.size()-1].size()){
                        cout<<"empty folder name\n";
                        return 0;
                    }
                    buffers[buffers.size()-1].erase(buffers[buffers.size()-1].begin()+lastrslash, buffers[buffers.size()-1].end());
                }
            }else{
                if(lastlslash!=string::npos){
                    if(!lastlslash) break;
                    if(lastlslash+1==buffers[buffers.size()-1].size()){
                        cout<<"empty folder name\n";
                        return 0;
                    }
                    buffers[buffers.size()-1].erase(buffers[buffers.size()-1].begin()+lastlslash, buffers[buffers.size()-1].end());
                }else{
                    break;
                }
            }
        }
        cout<<"such folder probably does not exist, creating...\n";
        for(int i=buffers.size()-2; i>-1; i--){
            CreateDirectory(buffers[i].c_str(), NULL);
        }
        if(nofilename){
            return 0;
        }
        plik.close();
        plik.open(track, ios::out | ios::binary);
        if(!plik.good()){
            cout<<"still cannot save file\n";
            return 0;
        }
    }
    plik<<input;
    plik.close();
    return 1;
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
bool copyToClipboard(string input, bool dkjvblhdsbcjlh){
    return copyToClipboard(input);
}

sf::RenderWindow window(sf::VideoMode(1200, 720), "downloader");
sf::Event event;
enum modes{urltyping, displaying, passwording, answering, writting, ftpconnect};
enum crypt{de, en, en_fromfile, text, salt, desalt, saving};
enum answers{t, n, wait};
modes mode=displaying;
crypt crypting;
answers answer=wait;
string url, uri, key, filename, filebody, buffer, header;
bool mouseButton, mask, TnI=0, selecting=0/*0=text, 1=image*/;
float spriteScale=1;
sf::Vector2f lastpos, utextdelta(0,40);
sf::Texture texture, maskTexture, downloadingTexture, downloadTexture, deleteTexture, decryptTexture, decryptingTexture, encryptTexture, encryptingTexture, resetTexture, saveTexture, savingTexture, textpointerTexture;
sf::Sprite sprite, maskSprite, downloadingSprite, downloadSprite, deleteSprite, decryptSprite, decryptingSprite, encryptSprite, encryptingSprite, resetSprite, saveSprite, savingSprite, textpointerSprite;
sf::Ftp ftp;
sf::Ftp::Response ftpresponse;
sf::Font mainfont, full_ascii;
sf::Text infotext, unknowntext;
sf::RectangleShape greybar(sf::Vector2f(1200, 40));
sf::Color bgcolor(50,50,50);
vector<sf::RectangleShape> selectfield;
int charsize=14, cursorpos=0, timer=0;
unsigned int selectchar[2];

void resetbuffers(){
    mode=displaying;
    system("title displaying");
    window.setTitle("downloader   mode(displaying)");
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
    if(selecting){
        selectchar[1]=cursorpos;

        if(selectchar[0]==selectchar[1]){
            selectfield.clear();
        }else{
            sf::Vector2f selcharpos[2];
            selcharpos[0]=unknowntext.findCharacterPos(selectchar[0]);
            selcharpos[1]=unknowntext.findCharacterPos(selectchar[1]);
            bool biggercharpos=(selectchar[0]<selectchar[1]);
            if(selcharpos[0].y==selcharpos[1].y){
                if(selectfield.size()!=1){
                    selectfield.clear();
                    selectfield.push_back(sf::RectangleShape());
                }
                selectfield[0].setPosition(selcharpos[!biggercharpos]);
                selectfield[0].setSize(sf::Vector2f(selcharpos[biggercharpos].x-selcharpos[!biggercharpos].x,charsize*4/3));
                selectfield[0].setFillColor(sf::Color(120,120,120));
            }else
            if(biggercharpos){
                int i1=selectchar[0], i2=filebody.find("\n", selectchar[0]), icount=1;
                if(!selectfield.size()){
                    selectfield.push_back(sf::RectangleShape());
                    selectfield[0].setFillColor(sf::Color(120,120,120));
                }
                if(selectfield[0].getGlobalBounds().left+selectfield[0].getGlobalBounds().width!=unknowntext.findCharacterPos(i2).x){
                    selectfield[0].setPosition(unknowntext.findCharacterPos(i1));
                    selectfield[0].setSize(sf::Vector2f(unknowntext.findCharacterPos(i2).x-unknowntext.findCharacterPos(i1).x,charsize*4/3));
                }
                while(1){
                    i1=(++i2);
                    i2=filebody.find("\n", i1);
                    if(selectfield.size()<=icount){
                        selectfield.push_back(sf::RectangleShape());
                        selectfield[icount].setFillColor(sf::Color(120,120,120));
                    }
                    if(i2>=selectchar[1]){
                        while(selectfield.size()>icount+1){
                            selectfield.pop_back();
                        }
                        selectfield[icount].setPosition(unknowntext.findCharacterPos(i1));
                        selectfield[icount].setSize(sf::Vector2f(unknowntext.findCharacterPos(cursorpos).x-unknowntext.findCharacterPos(i1).x,charsize*4/3));
                        break;
                    }
                    selectfield[icount].setPosition(unknowntext.findCharacterPos(i1));
                    selectfield[icount].setSize(sf::Vector2f(unknowntext.findCharacterPos(i2).x-unknowntext.findCharacterPos(i1).x,charsize*4/3));
                    icount++;
                }
            }
            else{
                int i1=selectchar[0], i2=filebody.rfind("\n", selectchar[0]-1), icount=1;
                if(!selectfield.size()){
                    selectfield.push_back(sf::RectangleShape());
                    selectfield[0].setFillColor(sf::Color(120,120,120));
                }
                selectfield[0].setPosition(unknowntext.findCharacterPos(i2+1));
                selectfield[0].setSize(sf::Vector2f(unknowntext.findCharacterPos(i1).x,charsize*4/3));
                while(1){
                    i1=i2;
                    i2=filebody.rfind("\n", i1-1);
                    if(selectfield.size()<=icount){
                        selectfield.push_back(sf::RectangleShape());
                        selectfield[icount].setFillColor(sf::Color(120,120,120));
                    }
                    if((i2<selectchar[1])||(i2==string::npos)){
                        while(selectfield.size()>icount+1){
                            selectfield.pop_back();
                        }
                        selectfield[icount].setPosition(unknowntext.findCharacterPos(i1));
                        selectfield[icount].setSize(sf::Vector2f(unknowntext.findCharacterPos(cursorpos).x-unknowntext.findCharacterPos(i1).x,charsize*4/3));
                        break;
                    }
                    selectfield[icount].setPosition(unknowntext.findCharacterPos(i2+1));
                    selectfield[icount].setSize(sf::Vector2f(unknowntext.findCharacterPos(i1).x,charsize*4/3));
                    icount++;
                }
            }
        }
    }else
    if(selectfield.size()){
        selectchar[0]=selectchar[1];
        selectfield.clear();
    }

}

int main()
{
    {   system("color 0a");
        system("title displaying");
        window.setTitle("downloader   mode(displaying)");
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
        saveTexture.loadFromFile("img/checkbox_save.bmp");
        saveSprite.setTexture(saveTexture);
        saveSprite.setPosition(200,0);
        savingTexture.loadFromFile("img/checkbox_saving.bmp");
        savingSprite.setTexture(savingTexture);
        savingSprite.setPosition(200,0);
        textpointerTexture.loadFromFile("img/pointer.bmp");
        textpointerSprite.setTexture(textpointerTexture);
        textpointerSprite.setPosition(utextdelta);
        textpointerSprite.setScale(charsize/12, (charsize*4)/3);
        greybar.setPosition(0,0);
        greybar.setFillColor(bgcolor);
        mainfont.loadFromFile("font.ttf");
        full_ascii.loadFromFile("SourceCodePro-Regular.ttf");
        infotext.setFont(mainfont);
        infotext.setColor(sf::Color(0,255,0));
        infotext.setCharacterSize(14);
        infotext.setPosition(240, 0);
        unknowntext.setFont(full_ascii);
        unknowntext.setColor(sf::Color(0,255,0));
        unknowntext.setCharacterSize(14);
        unknowntext.setPosition(utextdelta);
        header="downloader v1.3 by Aleksander Czajka\npress k for key biddings\n\n";
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
                            window.setTitle("downloader   mode(ftp)");
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
                                    sprite.setTexture(texture,1);
                                    cout<<"\ntype name \\/\n";
                                    infotext.setString("name=");
                                    mode=passwording;
                                    system("title passwording");
                                    window.setTitle("downloader   mode(passwording)");
                                    break;
                                }else{
                                    TnI=1;
                                    mode=writting;
                                    system("title writting");
                                    window.setTitle("downloader   mode(writting)");
                                    infotext.setString("Unknown format.");
                                }
                                unknowntext.setString(filebody);
                                cursorpos=0;
                            }else{
                                cout<<response.getStatus();
                                cout<<response.getBody();
                                mode=displaying;
                                system("title displaying");
                                window.setTitle("downloader   mode(displaying)");
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
                        if(filename.length()){cout<<"\n";
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
                                    plik.seekg(0, plik.end);
                                    if(buffer.max_size()<plik.tellg()+102400){
                                        cout<<"Sorry, file is too big to be read. Try to encrypt it using 'e'.\nMax file length is for now "<<buffer.max_size()-102400<<" bytes.\n";
                                        plik.close();
                                        unknowntext.setString(buffer);
                                        filebody="";
                                        TnI=1;
                                        system("title writting");
                                        window.setTitle("downloader   mode(writting)");
                                        mode=writting;
                                        crypting=text;
                                        cursorpos=0;
                                        break;
                                    }
                                    plik.seekg(0, plik.beg);
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
                                        sprite.setTexture(texture,1);
                                        unknowntext.setString(buffer);
                                        filebody=buffer;
                                        resetbuffers();
                                        TnI=0;
                                        system("title displaying");
                                        window.setTitle("downloader   mode(displaying)");
                                        mode=displaying;
                                    }else{
                                        unknowntext.setString(buffer);
                                        filebody=buffer;
                                        TnI=1;
                                        system("title writting");
                                        window.setTitle("downloader   mode(writting)");
                                        mode=writting;
                                        crypting=text;
                                    }
                                    cursorpos=0;
                                }else{
                                    cout<<"\nfile loading error\n";
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
                            }else
                            if(crypting==text){
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
                                    sprite.setTexture(texture,1);
                                    unknowntext.setString(filename);
                                    filebody=filename;
                                    resetbuffers();
                                    TnI=0;
                                    system("title displaying");
                                    window.setTitle("downloader   mode(displaying)");
                                    mode=displaying;
                                }else{
                                    unknowntext.setString(filename);
                                    filebody=filename;
                                    TnI=1;
                                    system("title writting");
                                    window.setTitle("downloader   mode(writting)");
                                    mode=writting;
                                }
                            }else
                            if(crypting==salt){
                                int seci=atoi(key.c_str());
                                if((seci<=filebody.length())&&(key.size())){
                                    srand(time(0));
                                    filebody.insert(seci,1,char(rand()%256));
                                }else
                                    cout<<"file or key is too short\n";
                                if(texture.loadFromMemory(&filebody[0], filebody.length())){
                                    sprite.setTexture(texture,1);
                                    unknowntext.setString(filebody);
                                    resetbuffers();
                                    TnI=0;
                                    system("title displaying");
                                    window.setTitle("downloader   mode(displaying)");
                                    mode=displaying;
                                }else{
                                    unknowntext.setString(filebody);
                                    TnI=1;
                                    system("title writting");
                                    window.setTitle("downloader   mode(writting)");
                                    mode=writting;
                                    crypting=text;
                                }
                                cursorpos=0;
                            }else
                            if(crypting==desalt){
                                int seci=atoi(key.c_str());
                                if((seci<filebody.length())&&(key.size())){
                                    filebody.erase(seci,1);
                                }else
                                    cout<<"file or key is too short\n";
                                if(texture.loadFromMemory(&filebody[0], filebody.length())){
                                    sprite.setTexture(texture,1);
                                    unknowntext.setString(filebody);
                                    resetbuffers();
                                    TnI=0;
                                    system("title displaying");
                                    window.setTitle("downloader   mode(displaying)");
                                    mode=displaying;
                                }else{
                                    unknowntext.setString(filebody);
                                    TnI=1;
                                    system("title writting");
                                    window.setTitle("downloader   mode(writting)");
                                    mode=writting;
                                    crypting=text;
                                }
                                cursorpos=0;
                            }else
                            if(crypting==saving){
                                if(key.length()){
                                    save(key, filebody);

                                }else
                                    cout<<"track is empty\n";
                                TnI=1;
                                system("title writting");
                                window.setTitle("downloader   mode(writting)");
                                mode=writting;
                                crypting=text;
                            }
                            infotext.setString(header);
                        }else{
                            filename=key;
                            key="";
                            if(filename.length()) cout<<"\ntype password \\/\n";
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
                            if(!filename.length()) cout<<(char*)(GetClipboardData(CF_TEXT));
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
                        if(!filename.length())
                            cout<<char(event.text.unicode);
                        else
                            cout<<'*';
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
                        key="";
                        cout<<"\ntype password \\/\n";
                        infotext.setString("password=");
                        crypting=text;
                    }else
                    if((event.text.unicode==22)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        if(OpenClipboard(0)){
                            if(selectchar[0]!=selectchar[1]){
                                if(selectchar[0]<selectchar[1]){
                                    unknowntext.setString(filebody.replace(selectchar[0], selectchar[1]-selectchar[0], (char*)(GetClipboardData(CF_TEXT))));
                                    cursorpos=1+(selectchar[1]=selectchar[0]);
                                }else{
                                    unknowntext.setString(filebody.replace(selectchar[1], selectchar[0]-selectchar[1], (char*)(GetClipboardData(CF_TEXT))));
                                    cursorpos=1+(selectchar[0]=selectchar[1]);
                                }
                            }else{
                            int i=filebody.length();
                                unknowntext.setString(filebody.insert(cursorpos, (char*)(GetClipboardData(CF_TEXT))));
                                i=filebody.length()-i;
                                cursorpos+=i;
                            }
                            textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                            centerText();
                            CloseClipboard();
                        }
                    }else
                    if((event.text.unicode==3)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        if(selectchar[0]==selectchar[1])
                            copyToClipboard(filebody);
                        else{
                            bool biggercharpos=(selectchar[0]<selectchar[1]);
                            copyToClipboard(filebody.substr(selectchar[!biggercharpos], selectchar[biggercharpos]-selectchar[!biggercharpos]), 1);
                        }
                    }else
                    if((event.text.unicode==1)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        mode=passwording;
                        filename="1";
                        key="";
                        cout<<"\ntype position of salt \\/\n";
                        infotext.setString("position=");
                        crypting=salt;
                    }else
                    if((event.text.unicode==18)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        mode=passwording;
                        filename="1";
                        key="";
                        cout<<"\ntype position of salt \\/\n";
                        infotext.setString("position=");
                        crypting=desalt;
                    }else
                    if((event.text.unicode==19)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                        mode=passwording;
                        filename="1";
                        key="";
                        cout<<"\ntype track\\/\n";
                        infotext.setString("track=");
                        crypting=saving;
                    }
                    else{
                        unsigned char etu=event.text.unicode;
                        if(etu==13)etu=10;
                        if(selectchar[0]!=selectchar[1]){
                            if(selectchar[0]<selectchar[1]){
                                unknowntext.setString(filebody.replace(selectchar[0], selectchar[1]-selectchar[0], 1, etu));
                                cursorpos=1+(selectchar[1]=selectchar[0]);
                            }else{
                                unknowntext.setString(filebody.replace(selectchar[1], selectchar[0]-selectchar[1], 1, etu));
                                cursorpos=1+(selectchar[0]=selectchar[1]);
                            }
                        }else{
                            unknowntext.setString(filebody.insert(cursorpos, 1, etu));
                            cursorpos++;
                        }

                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                    }
                    selecting=0;
                    centerText();
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
                        window.setTitle("downloader   mode(displaying)");
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
                    window.setTitle("downloader   mode(passwording)");
                }else
                if(event.text.unicode==101){
                    mode=passwording;
                    crypting=en_fromfile;
                    cout<<"\nencrypting from file:\ntype name\\/\n";
                    infotext.setString("name=");
                    system("title passwording");
                    window.setTitle("downloader   mode(passwording)");
                }else
                if(event.text.unicode==104){
                    cout<<"\ndownloading:\ntype URL\\/\n";
                    infotext.setString("URL=");
                    mode=urltyping;
                    crypting=en;
                    system("title urltyping");
                    window.setTitle("downloader   mode(urltyping)");
                }else
                if(event.text.unicode==107){
                    cout<<"console`s header shows current mode.\nh = http request (unfortunately doesn`t support https)\nd = decrypt from disc\ne = encrypt from disc and overwrite\nm = turns mask on/off\nc = clear screen\nr = reset position and scale of image\nw = writting mode\n   ctrl+q = en/de crypt text\n   ctrl+backspace = delete all text\n   ctrl+c = copy all text\n   ctrl+v = paste\n   ctrl+s = save as...\n   ctrl+a = add salt\n   ctrl+r = remove salt\nesc = back to display mode and clear all buffers\n\nencryption and decryption are the same operations because of algoritm used here.\n";
                }else
                if(event.text.unicode==109){
                    mask=!mask;
                }else
                if(event.text.unicode==119){
                    mode=writting;
                    TnI=1;
                    key="";
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
                        if((selectchar[0]!=selectchar[1])&&(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)))
                            cursorpos=selectchar[(selectchar[0]>selectchar[1])];
                        else
                            cursorpos--;
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        centerText();
                        timer=0;
                    }else
                    if((event.key.code==sf::Keyboard::Right)&&(cursorpos<filebody.length())){
                        if((selectchar[0]!=selectchar[1])&&(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)))
                            cursorpos=selectchar[(selectchar[0]<selectchar[1])];
                        else
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
                            if(findings[1]+1+findings[0]<cursorpos) cursorpos=findings[1]+1+findings[0];
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
                    }else
                    if((event.key.code==sf::Keyboard::LShift)&&(!selecting)){
                        selectchar[0]=selectchar[1]=cursorpos;
                        selecting=1;
                    }else
                    if(event.key.code==sf::Keyboard::Home){
                        cursorpos=0;
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        centerText();
                        timer=0;
                    }else
                    if(event.key.code==sf::Keyboard::End){
                        cursorpos=filebody.length();
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        centerText();
                        timer=0;
                    }
                }
            }else
            if(event.type==sf::Event::KeyReleased){
                if(event.key.code==sf::Keyboard::LShift){
                    selecting=0;
                    if(selectchar[0]>selectchar[1]){
                        unsigned int asdvfajhc=selectchar[0];
                        selectchar[0]=selectchar[1];
                        selectchar[1]=asdvfajhc;
                    }else
                    if(selectchar[0]==selectchar[1])
                        selectchar[0]=selectchar[1]=0;
                }
            }else
            if(event.type==sf::Event::MouseButtonPressed){
                mouseButton=1;
                if(mode==writting){
                    if(event.mouseButton.y<unknowntext.findCharacterPos(0).y){
                        cursorpos=0;
                    }else
                    if(event.mouseButton.y>unknowntext.findCharacterPos(filebody.length()-1).y+charsize*4/3){
                        cursorpos=filebody.length();
                    }else{
                        cursorpos=0;
                        while(event.mouseButton.y>unknowntext.findCharacterPos(cursorpos+1).y){
                            cursorpos=filebody.find('\n', cursorpos+1);
                            if(cursorpos==string::npos){
                                cursorpos=filebody.length();
                                break;
                            }
                        }
                        while(event.mouseButton.x<unknowntext.findCharacterPos(cursorpos).x){
                            cursorpos--;
                        }
                    }
                    selecting=1;
                    selectchar[1]=selectchar[0]=cursorpos;
                    textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                    centerText();
                    timer=0;
                }
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
                    window.setTitle("downloader   mode(urltyping)");
                }else
                if((event.mouseButton.x<deleteSprite.getPosition().x+deleteTexture.getSize().x)    &&(event.mouseButton.y<deleteSprite.getPosition().y+deleteTexture.getSize().y)    &&(event.mouseButton.x>=deleteSprite.getPosition().x)  &&(event.mouseButton.y>=deleteSprite.getPosition().y)){
                    texture.create(1,1);
                    sprite.setTexture(texture,1);
                    cout<<"\nimage cleared\n";
                }else
                if((event.mouseButton.x<decryptSprite.getPosition().x+decryptTexture.getSize().x)  &&(event.mouseButton.y<decryptSprite.getPosition().y+decryptTexture.getSize().y)  &&(event.mouseButton.x>=decryptSprite.getPosition().x) &&(event.mouseButton.y>=decryptSprite.getPosition().y)){
                    resetbuffers();
                    mode=passwording;
                    crypting=de;
                    cout<<"\ndecrypting:\ntype name\\/\n";
                    infotext.setString("name=");
                    system("title passwording");
                    window.setTitle("downloader   mode(passwording)");
                }else
                if((event.mouseButton.x<resetSprite.getPosition().x+resetTexture.getSize().x)      &&(event.mouseButton.y<resetSprite.getPosition().y+resetTexture.getSize().y)      &&(event.mouseButton.x>=resetSprite.getPosition().x)   &&(event.mouseButton.y>=resetSprite.getPosition().y)){
                    spriteScale=1;
                    sprite.setPosition(0,40);
                    sprite.setScale(1,1);
                    texture.loadFromMemory(&filebody[0], filebody.length());
                }else
                if((event.mouseButton.x<encryptSprite.getPosition().x+encryptTexture.getSize().x)  &&(event.mouseButton.y<encryptSprite.getPosition().y+encryptTexture.getSize().y)  &&(event.mouseButton.x>=encryptSprite.getPosition().x) &&(event.mouseButton.y>=encryptSprite.getPosition().y)){
                    resetbuffers();
                    mode=passwording;
                    crypting=en_fromfile;
                    cout<<"\nencrypting from file:\ntype name\\/\n";
                    infotext.setString("name=");
                    system("title passwording");
                    window.setTitle("downloader   mode(passwording)");
                }else
                if((event.mouseButton.x<saveSprite.getPosition().x+saveTexture.getSize().x)        &&(event.mouseButton.y<saveSprite.getPosition().y+saveTexture.getSize().y)        &&(event.mouseButton.x>=saveSprite.getPosition().x)    &&(event.mouseButton.y>=saveSprite.getPosition().y)){
                    mode=passwording;
                    filename="1";
                    key="";
                    cout<<"\ntype track\\/\n";
                    infotext.setString("track=");
                    crypting=saving;
                }
                if(selecting&&(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))){
                    selecting=0;
                }
            }else
            if(event.type==sf::Event::MouseMoved){
                if(mouseButton){
                    if(mode==writting){
                        if(event.mouseMove.y<unknowntext.findCharacterPos(0).y){
                            cursorpos=0;
                        }else
                        if(event.mouseMove.y>unknowntext.findCharacterPos(filebody.length()-1).y+charsize*4/3){
                            cursorpos=filebody.length();
                        }else{
                            cursorpos=0;
                            while(event.mouseMove.y>unknowntext.findCharacterPos(cursorpos+1).y){
                                cursorpos=filebody.find('\n', cursorpos+1);
                                if(cursorpos==string::npos){
                                    cursorpos=filebody.length();
                                    break;
                                }
                            }
                            while(event.mouseMove.x<unknowntext.findCharacterPos(cursorpos).x){
                                cursorpos--;
                            }
                        }
                        selectchar[1]=cursorpos;
                        textpointerSprite.setPosition(unknowntext.findCharacterPos(cursorpos));
                        centerText();
                        timer=0;
                    }else
                    if(mode==displaying)
                        sprite.setPosition(sprite.getPosition()-lastpos+sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
                }
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
                    for(int i=0; i<selectfield.size(); i++){
                         selectfield[i].move(0,event.mouseWheel.delta*20);
                    }
                    textpointerSprite.move(0,event.mouseWheel.delta*20);
                    unknowntext.setPosition(utextdelta);
                    timer=0;
                }
            }else
            if(event.type==sf::Event::Resized) window.setSize(sf::Vector2u(1200, 720));
        }

        timer++;
        if(timer>29)timer=0;

        window.clear(bgcolor);
        if(TnI){
            if(selectchar[0]!=selectchar[1])
                for(int i=0; i<selectfield.size(); ++i)
                    window.draw(selectfield[i]);
            window.draw(unknowntext);
            if(timer<15) window.draw(textpointerSprite);
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
        window.draw(saveSprite);
        if(mode!=displaying)
            if(crypting==en)              window.draw(downloadingSprite);
            else if(crypting==en_fromfile)window.draw(encryptingSprite);
            else if(crypting==de)         window.draw(decryptingSprite);
            else if(crypting==saving)     window.draw(savingSprite);
        window.draw(deleteSprite);
        window.draw(resetSprite);
        window.draw(infotext);
        window.display();
    }
    return 0;
}
