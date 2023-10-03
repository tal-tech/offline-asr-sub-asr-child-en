#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/base64.h"
#include "include/tlv_kaldi_dec_interface.h"
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
using namespace std;

struct ASRDecCfg {
    explicit ASRDecCfg(tlv_kaldi_dec_cfg *d) : data(d) {}
    ~ASRDecCfg() {
        if(data) {
            tlv_kaldi_dec_cfg_delete(data);
            data = nullptr;
        }
    }
    tlv_kaldi_dec_cfg *data;
};
struct ASRDec {
    explicit ASRDec(tlv_kaldi_dec *d) : data(d) {}
    ~ASRDec() {
        if(data) {
            tlv_kaldi_dec_delete(data);
            data = nullptr;
        }
    }

    tlv_kaldi_dec *data;
};
struct FastASRBodyInfo{//asr 服务的body 字段
        std::string     data;
        std::string     param;
        bool            kws{false};
    };
//std::mutex g_asr_mu;
std::shared_ptr<ASRDecCfg> g_asr_cfg(nullptr);
std::vector<std::shared_ptr<ASRDec>> g_asr_dec;
std::string  g_vad_rtc_param_str;
tlv_kaldi_dec *dec;
void Init()
{
    std::string cfg_dir = "../../../res";
    base::FilePath  fp(cfg_dir);
    if(!base::DirectoryExists(base::FilePath(cfg_dir))){
        //is not dir
        return ;
    }
    auto mod_file = fp.Append("cfg");//构造当前模型的cfg文件
    if(!base::PathExists(mod_file)){
        cout << mod_file.AsUTF8Unsafe() << " does not exist"<< endl;
        return ;
    }
    auto cfg_ptr = tlv_kaldi_dec_cfg_new(mod_file.value().c_str());
    if(!cfg_ptr){
        cout << " failed to init kaldi cfg:" << mod_file.value() << endl;
        // return false;
    }
    g_asr_cfg.reset(new ASRDecCfg(cfg_ptr));

}
std::shared_ptr<ASRDec> GetDecModule(){
    std::shared_ptr<ASRDec> dec;
    if(g_asr_dec.empty()){
        return std::make_shared<ASRDec>(tlv_kaldi_dec_new(g_asr_cfg->data));
    }else{
        dec = g_asr_dec.back();
        g_asr_dec.pop_back();
    }
    return dec;
}

void RetrieveModule(std::shared_ptr<ASRDec> dec)
{
    g_asr_dec.push_back(dec);
}

bool KaldiRecog(const FastASRBodyInfo &info,std::string &text, std::string &err_msg)
{
    text.clear();
    auto d = GetDecModule();
    do{
        if(!d){
            err_msg = "failed to init kaldi_dec";
            break;
        }

        if(info.kws){
            tlv_enable_module_dec(d->data, 1, 1);
        }else{
            tlv_enable_module_dec(d->data, 1, 0);
        }

        if(tlv_kaldi_dec_feed(d->data, (char *)info.data.c_str(), info.data.size(), 1)){
            err_msg = "failed to feed kaldi_dec";
            break;
        }
        //调用模型获取结果
        char * res_buf = nullptr;
        int re_len = -1;
        if(tlv_kaldi_dec_get_rslt(d->data, &res_buf, &re_len)){
            free(res_buf);
            err_msg = "failed to get kaldi_result";
            break;
        }

        if(res_buf && re_len > 0){
            text = res_buf;
        }else{
            err_msg = "empty kaldi_result";
            free(res_buf);
            break;
        }
        free(res_buf);
        tlv_kaldi_dec_reset(d->data);
        RetrieveModule(d);//回收dec
        return true;
    }while(false);
    tlv_kaldi_dec_reset(d->data);
    RetrieveModule(d);//回收dec
    return false;
}
std::string extractText(const std::string& input) {
    // 找到"text"的位置
    size_t textPos = input.find("\"text\":");
    
    if (textPos != std::string::npos) {
        // 找到冒号后的第一个双引号
        size_t startQuote = input.find("\"", textPos + 7);
        
        if (startQuote != std::string::npos) {
            // 找到双引号结束的位置
            size_t endQuote = input.find("\"", startQuote + 1);
            
            if (endQuote != std::string::npos) {
                // 提取并返回文本内容
                return input.substr(startQuote + 1, endQuote - startQuote - 1);
            }
        }
    }
    
    // 未找到匹配的内容
    return "";
}
int main()
{
    std::string text;//存储结果
    
    Init();
    FastASRBodyInfo info;
    string err_msg;
    std::string path = "../../../english.wav";
    std::ifstream audioFileStream(path, std::ios::binary);
    if (!audioFileStream) 
    {
        std::cerr << "Failed to open audio file." << std::endl;
        return 1;
    }
    std::string audioData((std::istreambuf_iterator<char>(audioFileStream)),std::istreambuf_iterator<char>());
    std::string b64Data;
    base::Base64Encode(audioData, &b64Data);
    //std::cout << b64Data;
    std::string data_str = "";
    // Base64解码
    base::Base64Decode(b64Data, &data_str);
    info.data = data_str;
    KaldiRecog(info, text, err_msg);
    cout << extractText(text) << endl;
    return 0;
}