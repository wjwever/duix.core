#pragma once
#include <map>
#include <string>

class config {
private:
  config() {};

public:
  static config *get();
  bool valid();
  std::string groupId = "";
  std::string apiKey = "";
  std::map<std::string, std::string> roles = {
      {"Andrew", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                 "dhp-tools/dhp-tools/651705983152197/61025/"
                 "651705983152197_ccf3256b2449c76e77f94276dffcb293.zip"},
      {"MoneyGod", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                   "dhp-tools/dhp-tools/651644573884485/61002/"
                   "651644573884485_2387469906049706416017f105e5340f.zip"},
      {"SuShi", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                "dhp-tools/dhp-tools/651660515688517/61011/"
                "651660515688517_825de648c30be80a89110dd0e63ecb3b.zip"},
      {"Eric", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
               "digital/model/1719193748558/airuike_20240409.zip"},
      {"ZiXuan", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                 "digital/model/1719194036608/zixuan_20240411v2.zip"},
      {"MingXuan", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                   "duix/digital/model/1719194633518/mingxuan_20240624.zip"},
      {"Arya", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
               "dhp-tools/dhp-tools/651637733658693/61000/"
               "651637733658693_2e0a4278a73411a2ff04ef1a849d2a6d.zip"},
      {"Shirley", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                  "dhp-tools/dhp-tools/651686686687301/61026/"
                  "651686686687301_846161843f9ffdaaeace716bf3436be5.zip"},
      {"Guanyin", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                  "dhp-tools/dhp-tools/651622691811397/60996/"
                  "651622691811397_ebe9bd2db8e26c1a7b07932b4a55c45c.zip"},
      {"Sophie", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                 "digital/model/1719193425133/sufei_20240409.zip"},
      {"MuRongXiao",
       "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/digital/"
       "model/1719193516102/murongxiao_20240410.zip"},
      {"ColdFlame", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                    "duix/digital/model/1719193451931/lengyan_20240407.zip"},
      {"Amelia", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                 "digital/model/1719193625986/amelia_20240411.zip"},
      {"ZhaoYa", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                 "digital/model/1719194234727/zhaoya_20240411.zip"},

      {"YiYao", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                "digital/model/1719194263441/yiyao_20240418.zip"},
      {"XinYan", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                 "digital/model/1719194373660/xinyan_20240411.zip"},
      {"XiaoXuan", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                   "duix/digital/model/1719194313414/xiaoxuan_20240418.zip"},
      {"SiYao", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                "digital/model/1719194450521/siyao_20240418.zip"},
      {"ShiYa", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/"
                "digital/model/1719194516880/shiya_20240409.zip"},
      {"DearSister", "https://digital-public.obs.cn-east-3.myhuaweicloud.com/"
                     "duix/digital/model/1719194007931/bendi1_0329.zip"}};
};
