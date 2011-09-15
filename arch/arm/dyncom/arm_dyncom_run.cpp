/*
 * The interface of dynamic compiled mode for ppc simulation
 *
 * 08/22/2010 Michael.Kang (blackfin.kang@gmail.com)
 */

#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Constant.h>
#include <llvm/Constants.h>
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Instructions.h"

#include <skyeye_dyncom.h>
#include <skyeye_types.h>
#include <skyeye_obj.h>
#include <skyeye.h>
#include <bank_defs.h>
#include <skyeye_pref.h>
#include <skyeye_symbol.h>
#include <dyncom/dyncom_llvm.h>
#include <skyeye_log.h>
#include <dyncom/tag.h>

#include <vector>

#include "arm_regformat.h"
#include <skyeye_ram.h>

#include "armdefs.h"
#include "memory.h"
#include "dyncom/memory.h"
#include "dyncom/frontend.h"
#include "arm_dyncom_translate.h"
#include "dyncom/defines.h"
#include "common/mmu/arm1176jzf_s_mmu.h"
#include "armmmu.h"

#include "dyncom/arm_dyncom_mmu.h"

#define LOG_IN_CLR	skyeye_printf_in_color

void arm_switch_mode(cpu_t *cpu);
//#define MAX_REGNUM 16
extern const char* arm_regstr[MAX_REG_NUM];
enum {
	ARM_DYNCOM_MCR = 2,
};


uint32_t get_end_of_page(uint32 phys_addr){
	const uint32 page_size = 4 * 1024;
	return (phys_addr + page_size) & (~(page_size - 1));
}

void cpu_set_flags_codegen(cpu_t *cpu, uint32_t f)
{
        cpu->dyncom_engine->flags_codegen = f;
}
static uint32_t int_icounter[] = {1953097, 2082332, 2211567, 2340802, 2470037, 2599272, 2728507, 2857742, 2986977, 3116212, 
				3245447, 3374682, 3503917, 3633152, 3762387, 3891622, 4020857, 4027664, 4033277, 4063047,
				4063047, 4149547, 4150163, 4157113, 4279332, 4408567, 4537802, 4667037, 4796272, 4925507,
				5054755, 5183977, 5313448, 5442447, 5571682, 5622630, 5683942, 5691077, 5698212, 5705347,
				5707073, 5718725, 5725860, 5732995, 5740130, 5744864, 5781941, 5789076, 5796320, 5803564,
				5810808, 5818052, 5825296, 5832540, 5834275, 5843597, 5848740, 5959407, 6089002, 6217877, 6347243, 6476347, 6605717, 6735282, 6864052, 6993771, 7122522, 7251757, 7380992, 7510522, 7598869, 7639463, 7770874, 7772600, 7844787, 7887190, 7897936, 8012410, 8019930, 8027173, 8156635, 8176024, 8190813, 8198333, 8291734, 8293460, 8414882, 8544117, 8673352, 8802660, 8931822, 9061057, 9190456, 9319527, 9448762, 9577997, 9707295, 9836467, 9965702, 10094937, 10224172, 10353407, 10482642, 10611884, 10741113, 10870347, 10999582, 11128817, 11258052, 11387287, 11516522, 11645799, 11775019, 11904227, 12033462, 12065781, 12160341, 12162699, 12291934, 12421169, 12550404, 12679639, 12808874, 12938109, 13067620, 13196579, 13325814, 13455049, 13584284, 13713519, 13842893, 13971989, 13990568, 14101548, 14233312, 14359695, 14488930, 14618165, 14747400, 14876695, 15005870, 15135105, 15264340, 15393575, 15522810, 15652045, 15781280, 15910782, 16039750, 16168985, 16298220, 16427455, 16556690, 16685925, 16815160, 16944395, 17073630, 17202865, 17332326, 17461335, 17590570, 17719805, 17849040, 17978275, 18107510, 18236745, 18365980, 18495215, 18624450, 18753685, 18882920, 19012155, 19141390, 19270625, 19370662, 19399861, 19480752, 19485909, 19511511, 19529099, 19658334, 19787569, 19916804, 20046039, 20175274, 20304509, 20331231, 20433745, 20562980, 20692215, 20821450, 20950685, 21079920, 21209155, 21338390, 21467625, 21596860, 21726095, 21855679, 21984565, 22113800, 22243035, 22372337, 22501505, 22630740, 22760134, 22889210, 23018445, 23147680, 23276915, 23406150, 23535385, 23664738, 23793855, 23923090, 24052328, 24181999, 24310795, 24440030, 24569265, 24698500, 24827735, 24956970, 25086580, 25215440, 25344675, 25473954, 25603145, 25732503, 25861615, 25990850, 26120085, 26249368, 26378555, 26507790, 26637025, 26766260, 26895495, 27025441, 27153965, 27283200, 27412435, 27541670, 27670905, 27800140, 27929375, 28058989, 28187845, 28317080, 28446315, 28575550, 28704785, 28834020, 28963255, 29092490, 29221725, 29350960, 29480195, 29609430, 29738665, 29867900, 29997135, 30126370, 30255605, 30384840, 30514075, 30643310, 30772545, 30902158, 31031027, 31160562, 31289485, 31418720, 31547955, 31677190, 31806425, 31935660, 32064895, 32194541, 32323365, 32452600, 32581835, 32711070, 32840305, 32969540, 33098775, 33228010, 33357245, 33486480, 33615715, 33744950, 33874267, 34003420, 34132655, 34261890, 34391125, 34520360, 34649595, 34778830, 34908065, 35037300, 35166535, 35295770, 35425005, 35554240, 35683475, 35812710, 35941945, 36071180, 36200415, 36329650, 36458885, 36588120, 36717355, 36846590, 36975825, 37105060, 37234295, 37363530, 37492765, 37622000, 37751235, 37880470, 38009705, 38138940, 38268175, 38397410, 38526645, 38655880, 38785459, 38914350, 39043637, 39172820, 39302055, 39431290, 39560733, 39689760, 39818995, 39948230, 40077465, 40206700, 40335935, 40465170, 40594405, 40723640, 40852875, 40982110, 41111345, 41240580, 41369815, 41499050, 41628285, 41757520, 41886818, 42015990, 42145225, 42274460, 42403695, 42532930, 42662165, 42791749, 42920635, 43049870, 43179326, 43308340, 43437575, 43566864, 43696045, 43825280, 43954515, 44083829, 44212985, 44342220, 44471455, 44600690, 44730019, 44859160, 44988395, 45117630, 45246865, 45376100, 45505344, 45634570, 45763805, 45893133, 46022275, 46151510, 46280745, 46409980, 46539215, 46668705, 46797685, 46926920, 47056155, 47185390, 47314625, 47443860, 47573095, 47702330, 47831565, 47960800, 48090059, 48219270, 48348505, 48477740, 48606975, 48736210, 48865445, 48994680, 49123915, 49253150, 49382385, 49511620, 49641190, 49770090, 49899975, 50028560, 50157795, 50287030, 50416265, 50545500, 50674735, 50803970, 50933205, 51062440, 51191675, 51320910, 51450145, 51579380, 51708615, 51838099, 51967085, 52096371, 52225555, 52354822, 52484025, 52613459, 52742495, 52871797, 53000965, 53130200, 53259435, 53388670, 53517905, 53647140, 53776375, 53905610, 54035065, 54164080, 54293315, 54422550, 54551785, 54681020, 54810255, 54939490, 55068725, 55198279, 55327195, 55456430, 55585665, 55715335, 55844135, 55973370, 56078583, 56102642, 56231841, 56361076, 56490311, 56572284, 56619547, 56681347, 56749240, 56792226, 56878072, 56902193, 57007341, 57136568, 57265725, 57395062, 57524195, 57653430, 57782665, 57911900, 58041135, 58170370, 58299605, 58428840, 58558075, 58687317, 58816545, 58945780, 59075015, 59204250, 59333485, 59462720, 59591955, 59721200, 59850425, 59931078, 59979661, 60108896, 60238174, 60367402, 60496601, 60625836, 60755171, 60884306, 61013541, 61142776, 61272011, 61307646, 61401247, 61497166, 61514554, 61530484, 61660095, 61720814, 61766648, 61788956, 61819158, 61844037, 61856793, 61918201, 62047429, 62176676, 62305899, 62435145, 62564375, 62693611, 62822848, 62952078, 63081315, 63210545, 63339782, 63469014, 63598249, 63727484, 63856719, 63985954, 64115189, 64244434, 64373671, 64502901, 64632138, 64761368, 64890605, 65019835, 65149072, 65278304, 65407539, 65536774, 65666009, 65795244, 65924479, 66053724, 66182949, 66312184, 66441419, 66570654, 66699889, 66829124, 66958359, 67087604, 67216841, 67346071, 67475308, 67604538, 67733769, 67863013, 67992250, 68121480, 68250717, 68379947, 68509184, 68638414, 68767651, 68896884, 69026119, 69155354, 69284589, 69413836, 69543059, 69672303, 69801540, 69930770, 70060007, 70189237, 70318474, 70447704, 70576941, 70706174, 70835409, 70964644, 71093879, 71223126, 71352349, 71481593, 71610830, 71740060, 71869297, 71998527, 72127764, 72256994, 72386229, 72515464, 72644699, 72773944, 72903181, 73032411, 73161648, 73290878, 73420115, 73549345, 73678582, 73807814, 73937049, 74066284, 74195519, 74324765, 74453989, 74583232, 74712469, 74841699, 74970936, 75100166, 75229403, 75358634, 75487870, 75617104, 75746339, 75875574, 76004809, 76134055, 76263279, 76392522, 76521759, 76650989, 76780226, 76909456, 77038693, 77167924, 77297160, 77426394, 77555629, 77684864, 77814099, 77943345, 78072569, 78201812, 78331049, 78460279, 78589516, 78718746, 78847983, 78977214, 79106453, 79235693, 79364930, 79494160, 79623397, 79752627, 79881864, 80011097, 80140334, 80269564, 80398801, 80528034, 80657269, 80786504, 80915739, 81044986, 81174209, 81303453, 81432690, 81561920, 81691157, 81820387, 81949624, 82078854, 82208091, 82337324, 82466559, 82595794, 82725029, 82854276, 82983499, 83112743, 83241980, 83371210, 83500447, 83629677, 83758914, 83888144, 84017381, 84146614, 84275849, 84405084, 84534319, 84663566, 84792789, 84922033, 85051270, 85180500, 85309737, 85438967, 85568204, 85697434, 85826671, 85955904, 86085139, 86214374, 86343609, 86472856, 86602079, 86731323, 86860560, 86989790, 87119027, 87248257, 87377494, 87506724, 87635961, 87765194, 87894429, 88023664, 88152899, 88282146, 88411369, 88540613, 88669850, 88799080, 88928317, 89057547, 89186784, 89316014, 89445251, 89574484, 89703719, 89832954, 89962189, 90091436, 90220659, 90349903, 90479140, 90608370, 90737607, 90866837, 90996074, 91125304, 91254541, 91383774, 91513009, 91642244, 91771479, 91900726, 92029949, 92159185, 92288422, 92417654, 92546889, 92676124, 92805359, 92934594, 93063829, 93193074, 93322311, 93451541, 93580778, 93710008, 93839245, 93968475, 94097712, 94226944, 94356179, 94485414, 94614649, 94743884, 94873119, 95002364, 95131601, 95260831, 95390068, 95519298, 95648535, 95777765, 95907002, 96036234, 96165469, 96294704, 96423939, 96553174, 96682409, 96811654, 96940891, 97070121, 97199358, 97328588, 97457825, 97587055, 97716292, 97845524, 97974759, 98103994, 98233229, 98362464, 98491699, 98620944, 98750181, 98879411, 99008648, 99137878, 99267115, 99396345, 99525582, 99654814, 99784049, 99913284, 100042519, 100171754, 100300989, 100430234, 100559471, 100688701, 100817938, 100947168, 101076405, 101205635, 101334872, 101464104, 101593339, 101722574, 101851809, 101981044, 102110279, 102239524, 102368761, 102497991, 102627228, 102756458, 102885695, 103014925, 103144162, 103273394, 103402629, 103531864, 103661099, 103790334, 103919569, 104048814, 104178051, 104307281, 104436518, 104565748, 104694985, 104824215, 104953455, 105082695, 105211919, 105341162, 105470399, 105599629, 105728866, 105858096, 105987333, 106116564, 106245800, 106375034, 106504269, 106633504, 106762739, 106891985, 107021209, 107150450, 107279687, 107408917, 107538154, 107667384, 107796621, 107925854, 108055089, 108184324, 108313559, 108442806, 108572029, 108701273, 108830510, 108959740, 109088977, 109218207, 109347444, 109476674, 109605911, 109735144, 109864379, 109993614, 110122849, 110252096, 110381319, 110510563, 110639800, 110769030, 110898267, 111027497, 111156734, 111285964, 111415201, 111544434, 111673669, 111802904, 111932139, 112061386, 112190609, 112319853, 112449090, 112578320, 112707557, 112836787, 112966024, 113095254, 113224491, 113353724, 113482959, 113612194, 113741429, 113870676, 113999899, 114129143, 114258380, 114387610, 114516847, 114646077, 114775314, 114904544, 115033781, 115163014, 115292249, 115421484, 115550719, 115679966, 115809189, 115938433, 116067670, 116196900, 116326137, 116455367, 116584604, 116713834, 116843071, 116972304, 117101539, 117230774, 117360009, 117489256, 117618479, 117747723, 117876949, 118006184, 118135419, 118264654, 118393889, 118523124, 118652359, 118781604, 118910841, 119040071, 119169308, 119298538, 119427775, 119557005, 119686242, 119815474, 119944709, 120073944, 120203179, 120332414, 120461649, 120590894, 120720131, 120849361, 120978598, 121107828, 121237065, 121366295, 121495532, 121624764, 121753999, 121883234, 122012469, 122141704, 122270939, 122400184, 122529421, 122658651, 122787888, 122917118, 123046355, 123175585, 123304822, 123434054, 123563289, 123692524, 123821759, 123950994, 124080229, 124209474, 124338711, 124467941, 124597178, 124726408, 124855645, 124984875, 125114112, 125243344, 125372579, 125501814, 125631049, 125760284, 125889519, 126018764, 126148001, 126277231, 126406468, 126535698, 126664935, 126794165, 126923402, 127052634, 127181869, 127311104, 127440339, 127569574, 127698809, 127828054, 127957291, 128086521, 128215758, 128344988, 128474225, 128603455, 128732692, 128861924, 128991159, 129120394, 129249629, 129378864, 129508099, 129637344, 129766581, 129895811, 130025048, 130154278, 130283515, 130412745, 130541982, 130671214, 130800452, 130929692, 131058929, 131188154, 131317389, 131446624, 131575859, 131705104, 131834341, 131963571, 132092808, 132222038, 132351275, 132480513, 132609750, 132738980, 132868217, 132997447, 133126684, 133255914, 133385151, 133514384, 133643619, 133772854, 133902089, 134031336, 134160559, 134289803, 134419040, 134548270, 134677507, 134806737, 134935974, 135065204, 135194441, 135323674, 135452909, 135582144, 135711379, 135840626, 135969849, 136099093, 136228330, 136357560, 136486797, 136616027, 136745264, 136874494, 137003729, 137132964, 137262199, 137391446, 137520669, 137649913, 137779150, 137908380, 138037617, 138166847, 138296084, 138425314, 138554551, 138683784, 138813019, 138942254, 139071489, 139200736, 139329959, 139459203, 139588440, 139717670, 139846907, 139976137, 140105374, 140234604, 140363839, 140493074, 140622309, 140751544, 140880779, 141010024, 141139261, 141268491, 141397728, 141526958, 141656195, 141785425, 141914662, 142043894, 142173129, 142302364, 142431599, 142560834, 142690069, 142819314, 142948551, 143077781, 143207018, 143336248, 143465485, 143594715, 143723955, 143853195, 143982419, 144111662, 144240899, 144370129, 144499366, 144628596, 144757833, 144887064, 145016300, 145145534, 145274769, 145404004, 145533239, 145662485, 145791709, 145920952, 146050189, 146179419, 146308656, 146437886, 146567123, 146696354, 146825590, 146954824, 147084059, 147213294, 147342529, 147471775, 147600999, 147730242, 147859479, 147988709, 148117946, 148247176, 148376413, 148505644, 148634880, 148764114, 148893349, 149022584, 149151819, 149281065, 149410289, 149539532, 149668769, 149797999, 149927236, 150056466, 150185703, 150314934, 150444170, 150573404, 150702639, 150831874, 150961109, 151090355, 151219579, 151348822, 151478059, 151607289, 151736526, 151865756, 151994993, 152124224, 152253460, 152382694, 152511929, 152641164, 152770399, 152899645, 153028869, 153158112, 153287349, 153416579, 153545816, 153675046, 153804283, 153933514, 154062750, 154191984, 154321219, 154450454, 154579689, 154708935, 154838159, 154967402, 155096639, 155225869, 155355106, 155484336, 155613573, 155742804, 155872040, 156001274, 156130509, 156259744, 156388979, 156518225, 156647449, 156776684, 156905921, 157035154, 157164389, 157293624, 157422859, 157552094, 157681329, 157810564, 157939799, 158069045, 158198269, 158327512, 158456749, 158585979, 158715216, 158844446, 158973683, 159102914, 159232150, 159361384, 159490619, 159619854, 159749089, 159878335, 160007559, 160136802, 160266039, 160395269, 160524506, 160653736, 160782973, 160912204, 161041440, 161170674, 161299909, 161429144, 161558379, 161687625, 161816849, 161946092, 162075329, 162204559, 162333796, 162463026, 162592263, 162721494, 162850730, 162979964, 163109199, 163238434, 163367669, 163496915, 163626139, 163755382, 163884619, 164013849, 164143086, 164272316, 164401553, 164530784, 164660020, 164789254, 164918489, 165047724, 165176959, 165306205, 165435429, 165564672, 165693909, 165823139, 165952376, 166081606, 166210843, 166340074, 166469310, 166598544, 166727779, 166857014, 166986249, 167115495, 167244719, 167373962, 167503199, 167632429, 167761666, 167890896, 168020133, 168149364, 168278600, 168407834, 168537069, 168666304, 168795539, 168924785, 169054009, 169183252, 169312489, 169441719, 169570959, 169700184, 169829419, 169958666, 170087889, 170217133, 170346370, 170475600, 170604837, 170734067, 170863304, 170992534, 171121771, 171251004, 171380239, 171509474, 171638709, 171767956, 171897179, 172026423, 172155660, 172284890, 172414127, 172543357, 172672594, 172801824, 172931061, 173060294, 173189529, 173318764, 173447999, 173577244, 173706481, 173835711, 173964948, 174094178, 174223415, 174352645, 174481882, 174611114, 174740349, 174869584, 174998819, 175128054, 175257289, 175386534, 175515771, 175645001, 175774238, 175903468, 176032705, 176161935, 176291172, 176420404, 176549639, 176678874, 176808109, 176937344, 177066579, 177195824, 177325061, 177454291, 177583528, 177712758, 177841995, 177971225, 178100462, 178229694, 178358929, 178488164, 178617399, 178746634, 178875869, 179005114, 179134351, 179263581, 179392818, 179522048, 179651285, 179780515, 179909752, 180038984, 180168219, 180297454, 180426689, 180555924, 180685159, 180814404, 180943641, 181072871, 181202108, 181331338, 181460575, 181589805, 181719042, 181848274, 181977509, 182106744, 182235979, 182365214, 182494449, 182623686, 182752923, 182882154, 183011390, 183140624, 183269859, 183399094, 183528329, 183657575, 183786799, 183916042, 184045279, 184174509, 184303746, 184432976, 184562213, 184691444, 184820680, 184949914, 185079149, 185208384, 185337619, 185466865, 185596089, 185725332, 185854569, 185983799, 186113036, 186242266, 186371503, 186500734, 186629970, 186759204, 186888439, 187017674, 187146909, 187276155, 187405379, 187534622, 187663859, 187793089, 187922326, 188051556, 188180793, 188310024, 188439260, 188568494, 188697729, 188826964, 188956199, 189085445, 189214669, 189343912, 189473149, 189602379, 189731616, 189860846, 189990083, 190119314, 190248550, 190377784, 190507019, 190636254, 190765489, 190894735, 191023959, 191035143, 191041712, 191050198, 191073517, 191153466, 191232608, 191282434, 191337391, 191384457, 191411671, 191540906, 191670340, 191683407, 191687942, 191713771, 191723574, 191733402, 191799381, 191928616, 192057851, 192187086, 192316321, 192445556, 192574791, 192704026, 192833261, 192962496, 193038724, 193094564, 193109403, 193121520, 193220969, 193350204, 193479439, 193608674, 193737909, 193867144, 193996379, 194125614, 194254849, 194384084, 194421783, 194516787, 195945227, 195946838, 195957286, 196064220, 196193367, 196265871, 196322603, 196451838, 196665874, 196667742, 199350864, 199569091, 199570916, 202254038, 202272205, 202396610, 202478249, 202482191, 202525847, 202570719, 202572870, 202655084, 202785298, 202833367, 202843533, 202846062, 202877986, 202886130, 202895905, 202913560, 202964865, 203029423, 203042797, 203066728, 203265728, 203267584, 205950706, 206010148, 206015185, 206076316, 206140940, 206144422, 206168408, 206385812, 206387722, 209070844, 209284641, 209286433, 211969555, 212174274, 214859427, 215064146, 217749299, 217954018, 220639171, 220853080, 220854872, 223537994, 223751893, 223753685, 226436807, 226650706, 226652498, 229335620, 229549519, 229551311, 232234433, 232249907, 232450566, 232452367, 235135489, 236494650, 236496451, 236514444, 236560841, 236624012, 236643681, 236690397, 236755021, 236772918, 236820755, 236883893, 236902155, 236951155, 237014260, 237031392, 237079994, 237143264, 237160629, 237211273, 237274804, 237289869, 237340838, 237408556, 237419103, 237474632, 237538004, 237548340, 237604137, 237669028, 237677577, 237735746, 237799118, 237806814, 237869516, 237932888, 237936051, 238007020, 238065287, 238079502, 238144676, 238194524, 238210376, 238274445, 238323761, 238341671, 238405109, 238452998, 238471152, 238534557, 238582296, 238604805, 238668276, 238711523, 238734319, 238799853, 238840709, 238865887, 238929325, 238969946, 238995368, 239060292, 239099183, 239126326, 239189863, 239228420, 239257433, 239321502, 239357657, 239387599, 239451037, 239486894, 239518599, 239582004, 239616131, 239648038, 239713028, 239745368, 239781865, 239845879, 239874605, 239911979, 239976870, 240003842, 240042946, 240106318, 240133079, 240174003, 240237375, 240262316, 240303451, 240367487, 240391553, 240435169, 240498541, 240520790, 240564617, 240629508, 240650027, 240695641, 240759013, 240779264, 240825089, 240908502, 241007619, 241037736, 241166971, 241248064, 241296207, 241425442, 241486808, 241550293, 241554679, 241683914, 241789178, 241813150, 241942385, 242027913, 242071621, 242200856, 242266542, 242329994, 242459327, 242569826, 242588563, 242717798, 242811300, 242847034, 242913663, 242977258, 242978923, 243043527, 243105507, 243110927, 243174742, 243234744, 243241715, 243305362, 243363981, 243373081, 243436577, 243493218, 243502900, 243570475, 243622455, 243636843, 243700339, 243751692, 243767463, 243832489, 243880929, 243898708, 243962160, 244010166, 244029724, 244093344, 244139403, 244159500, 244222952, 244268641, 244291519, 244354971, 244397877, 244421127, 244485930, 244527114, 244552344, 244615796, 244656351, 244681955, 244749621, 244785588, 244816789, 244880263, 244914825, 244947833, 245011453, 245044062, 245077666, 245141118, 245173394, 245208793, 245272245, 245302536, 245339328, 245404131, 245431773, 245470455, 245533907, 245561010, 245600647, 245665618, 245690247, 245731774, 245795226, 245819484, 245863660, 245924239, 245948721, 246077956, 246207191, 246336426, 246465661, 246594896, 246724302, 246853649, 246982601, 247111838, 247241071, 247370306, 247499541, 247628776, 247758011, 247887246, 248016481, 248081355, 248088355, 248089382, 248101774, 248121718, 248135268, 248145722, 248274957, 248404190, 248533420, 248662662, 248791897, 248913682, 248915601, 248917112, 248917789, 249022638};

static int flush_current_page(cpu_t *cpu);
#if __FOLLOW_MODE__
static uint32_t arm_get_id_from_string(char *reg_name, bool print_regname)
{
	if (print_regname) {
	//	printf("in %s\n", __FUNCTION__);
		printf("reg_name is %s\n", reg_name);
	}
        int i = 0;
        for (i = 0; i < MAX_REG_NUM; i ++) {
                if (0 == strcmp(arm_regstr[i], reg_name))
                        return i;
        }
}

static int last_idx = 0;
void update_int_array(cpu_t *cpu, uint32_t icounter)
{
	if (icounter > int_icounter[last_idx]) {
//		last_idx ++;
		int_icounter[last_idx] = icounter;
	}
}

#define PRINT_LOG 0
uint32_t follow_mode(cpu_t *cpu)
{
        static uint32_t wait_one_step = 1;
	static int adjust_pc = 0;
        char reg_name[20];
        static char string[100] = {0};
        int i = 0;
        uint32_t val = 0;
        uint32_t idx = 0;
        static uint32_t last_pc = 0;
        bool sw = true;
	static bool print_regname = false;
        arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	if (adjust_pc) {
		adjust_pc = 0;
		goto begin;
	}
        if (wait_one_step == 0) {
                wait_one_step = 1;
                last_pc = cpu->f.get_pc(cpu, cpu->rf.grf);
                return 0;
        }
        if (string[0] == 0) {
                fscanf(cpu->src_log, "%s", string);
        }
#if PRINT_LOG
        printf("%s\n", string);
#endif
        if (string[0] != 'P' && string[1] != 'C') {
                printf("log format is not wrong!\n");
#if PRINT_LOG
                printf("%s\n", string);
#endif
                exit(-1);
        }
        val = strtol(&string[3], NULL, 16);
        if (val == 0) {
                fscanf(cpu->src_log, "%s", string);
                val = strtol(&string[3], NULL, 16);
//                return;
        }
        last_pc = cpu->f.get_pc(cpu, cpu->rf.grf);
        if (val != last_pc) {
		if (val == 0xffff0018) {
			cpu->check_int_flag = 1;
			update_int_array(cpu, cpu->icounter);
			adjust_pc = 1;
			return 1;
		} else if (last_pc == 0xffff000c) {
			do {
				fscanf(cpu->src_log, "%s", string);
			} while (string[0] != 'P' && string[1] != 'C');
			val = strtol(&string[3], NULL, 16);
			if (val != last_pc) {
				printf("try again, but pc is still wrong.\n");
				exit(-1);
			}
		} else {
	//                printf("pc is wrong.\n");
			LOG_IN_CLR(RED, "pc is wrong\n");
	//                printf("dyncom mode pc is %x\n", core->Reg[15]);
			LOG_IN_CLR(BLUE, "dyncom mode pc is %x\n", core->Reg[15]);
			LOG_IN_CLR(CYAN, "dyncom mode phys_pc is %x\n", core->phys_pc);
			LOG_IN_CLR(LIGHT_RED, "interpreter mode is %x\n", val);
			LOG_IN_CLR(PURPLE, "icounter is %d\n", cpu->icounter);
			LOG_IN_CLR(RED, "adjust pc...\n");
//		core->Reg[15] = val;
//		flush_current_page(cpu);
//		return 1;
			exit(-1);
		}
        }
begin:
        fscanf(cpu->src_log, "%s", string);
#if PRINT_LOG
        printf("%s\n", string);
#endif
//	if (cpu->icounter >= 163630) {
//		print_regname = true;
//	}
        while(string[0] != 'P' && string[1] != 'C') {
                while(string[i] != ':') i++;
                string[i] = '\0';
                idx = arm_get_id_from_string(string, print_regname);
#if PRINT_LOG
                printf("idx is %d\n", idx);
#endif
                string[i] = ':';
                val = strtol(&string[i + 1], NULL, 16);
#if PRINT_LOG
                printf("val is %x\n", val);
#endif
                if (idx < 15 && core->Reg[idx] != val) { //&& ((val & 0xf) != 3)) {
			if (core->Reg[15] == 0xc0020ab0 && cpu->icounter > 237538020) {
				core->Reg[idx] = val;
			}
                        if (sw) {
                                printf("addr : %x\n", last_pc);
                                sw = false;
                        }
			if (idx != 16) {
				printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			}
                        printf("R%d's value is wrong.\n", idx);
                        printf("R%d wrong value : %x right value: %x\n", idx, core->Reg[idx], val);
                        if(idx != 15) {
                                fprintf(cpu->fm_log, "%x instr implementation is wrong\n", last_pc);
                        }
                        //core->Reg[idx] = val;
			printf("icounter is %lld\n", cpu->icounter);
                }
		i = 0;
                fscanf(cpu->src_log, "%s", string);
#if PRINT_LOG
                printf("%s\n", string);
#endif
        }
	return 0;
}
#endif
struct symbolInfo *symbol_info_head = NULL;

void add_list_tail(struct symbolInfo *list)
{
        static struct symbolInfo *symbol_info_tail = NULL;
        if(!symbol_info_head) {
                symbol_info_head = symbol_info_tail = list;
                return;
        }
        symbol_info_tail->next = list;
        symbol_info_tail = list;
}

uint8_t *store_string_info(char *info, size_t size)
{
        static uint32_t max_vol = 0x1000;
        static uint32_t offset = 0;
        static uint32_t remain = 0x1000;
        static uint8_t *repo = NULL;

        uint8_t *str = NULL;
        uint8_t *new_repo = NULL;
        struct symbolInfo *item = NULL;

        //printf("%s, %d, %d\n", info, size, remain);
        if (repo == NULL) {
                repo = (uint8_t *)malloc(max_vol);
                printf("allocate %d bytes.\n", max_vol);
        }
        if (remain < size) {
                new_repo = (uint8_t *)malloc(max_vol * 2);
                printf("allocate %d bytes.\n", max_vol * 2);
                memcpy(new_repo, repo, offset);
                for (item = symbol_info_head; item; item = item->next) {
                        //printf("symbol : %s\taddress : %x\n", item->name, item->address);
                        item->name = new_repo + ((uint8_t *)item->name - (uint8_t *)repo);
                }
                free(repo);
                repo = new_repo;
                new_repo = NULL;
                remain += max_vol;
                max_vol *= 2;
        }
        str = repo + offset;
        memcpy(repo + offset, info, size);
        repo[offset + size] = '\0';
        offset += size;
        remain -= size;
        return str;
}
struct symbolInfo *alloc_symbol_info(uint8_t *str, uint32_t address)
{
        struct symbolInfo *item = (struct symbolInfo *)malloc(sizeof(struct symbolInfo));
        if (item == NULL) {
                printf("Can't allocate more memory in %s\n", __FUNCTION__);
                exit(-1);
        }
        item->next = NULL;
        item->name = str;
        item->address = address;
        return item;
}

struct symbolInfo *search_symbol_info_by_addr(uint32_t address)
{
        struct symbolInfo *prev = NULL, *item = NULL;
        for (item = symbol_info_head; item; item = item->next) {
                if(address == item->address) {
                        return item;
                } else if(address > item->address){
                        prev = item;
                        continue;
                } else {
                        return prev;
                }
        }
        printf("Can not found the address 0x%x in System.map.\n", address);
        //exit(-1);
        return NULL;
}

void print_func_name(uint32_t address)
{
        static struct symbolInfo *last_found = NULL;
        static uint32_t last_address = 0;
        struct symbolInfo *new_found = NULL;
        new_found = search_symbol_info_by_addr(address);
        if (new_found == NULL) {
                return;
        }
        if (last_found != new_found) {
                if (last_found) {
                        LOG_IN_CLR(LIGHT_RED, "exit function %s 0x%x\n", last_found->name, last_address);
                }
                printf("%s\n", new_found->name);
                last_found = new_found;
                last_address = address;
        } else {
		last_address = address;
	}
}

void load_symbol_from_sysmap()
{
        char symbol_address[100];
        char symbol_name[100];
        char type = 0;
        char *str = NULL;
        struct symbolInfo *item = NULL;
        int i = 0;

        uint32_t address = 0;
        FILE *sysmap = fopen("/home/myesis/linux-2.6.35.y/System.map", "r");

        do {
                    if (3 != fscanf(sysmap, "%s %c %s", symbol_address, &type, symbol_name)) break;
                    address = strtol(symbol_address, NULL, 16);
                    while (symbol_name[i] != '\0') {
                            //printf("%c\n", symbol_name[i]);
                            i++;
                    }
                    //printf("symbol:%s\taddress:%x\tsize:%d\n", symbol_name, address, i);
                    str = (char *)store_string_info(symbol_name, i + 1);
                    item = alloc_symbol_info((uint8_t *)str, address);
                    add_list_tail(item);
        } while (1);
        for (item = symbol_info_head; item; item = item->next) {
                printf("symbol : %s\taddress : %x\n", item->name, item->address);
        }
}


uint32_t is_int_in_interpret(cpu_t *cpu)
{
	static int hit = 0;
	int curr_idx = last_idx;
	int length = sizeof(int_icounter) / sizeof(uint32_t);
	for (; curr_idx < length; curr_idx ++) {
		if (cpu->icounter < int_icounter[curr_idx]) {
			return 0;
		}
		if (int_icounter[curr_idx] == cpu->icounter) {
			last_idx = curr_idx;
			return 1;
		}
	}
	if (last_idx == length) {
		last_idx --;
	}
	return 0;
}


static cpu_flags_layout_t arm_flags_layout[4] ={{3,'N',"NFLAG"},{2,'Z',"ZFLAG"},{1,'C',"CFLAG"},{0,'V',"VFLAG"}} ;
/* physical register for arm archtecture */
static void arch_arm_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	arm_opc_func_init();
	// Basic Information
	info->name = "arm"; info->full_name = "arm_dyncom";

	// This architecture is biendian, accept whatever the
	// client wants, override other flags.
	info->common_flags &= CPU_FLAG_ENDIAN_MASK;
	/* set the flag of save pc */
	cpu->info.common_flags |= CPU_FLAG_SAVE_PC;

	info->delay_slots = 0;
	// The byte size is 8bits.
	// The word size is 32bits.
	// The float size is 64bits.
	// The address size is 32bits.
	info->byte_size = 8;
	info->word_size = 32;
	info->float_size = 64;
	info->address_size = 32;
	// There are 16 32-bit GPRs
	info->register_count[CPU_REG_GPR] = 19;
	info->register_size[CPU_REG_GPR] = info->word_size;
	// There is also 1 extra register to handle PSR.
	//info->register_count[CPU_REG_XR] = PPC_XR_SIZE;
	info->register_count[CPU_REG_XR] = MAX_REG_NUM - 19;
	//info->register_count[CPU_REG_XR] = 0;
	info->register_size[CPU_REG_XR] = 32;
	//info->register_count[CPU_REG_SPR] = MAX_REG_NUM - PHYS_PC;
	info->register_count[CPU_REG_SPR] = 0;
	info->register_size[CPU_REG_SPR] = 32;
	info->psr_size = 32;
	info->flags_count = 4;
	info->flags_layout = arm_flags_layout;

	cpu->redirection = false;

	//debug
	cpu_set_flags_debug(cpu, 0
	//	| CPU_DEBUG_PRINT_IR
	//	| CPU_DEBUG_LOG
               );
        cpu_set_flags_codegen(cpu, CPU_CODEGEN_TAG_LIMIT 
			      | CPU_CODEGEN_VERIFY
			      );
	/* Initilize different register set for different core */

//	set_memory_operator(arch_arm_read_memory, arch_arm_write_memory);
//	arm_dyncom_mcr_init(cpu);
}

static void
arch_arm_done(cpu_t *cpu)
{
	//free(cpu->rf.grf);
}

static addr_t
arch_arm_get_pc(cpu_t *, void *reg)
{
	unsigned int *grf =(unsigned int *) reg;
	return grf[15];
}

static uint64_t
arch_arm_get_psr(cpu_t *, void *reg)
{
	return 0;
}

static int
arch_arm_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	return (0);
}

static int arch_arm_disasm_instr(cpu_t *cpu, addr_t pc, char* line, unsigned int max_line){
	return 0;
}
static int arch_arm_translate_loop_helper(cpu_t *cpu, addr_t pc, BasicBlock *bb_ret, BasicBlock *bb_next, BasicBlock *bb, BasicBlock *bb_zol_cond){
	return 0;
}

static void arch_arm_emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
	Value *nzcv = LSHR(AND(LOAD(cpu->ptr_gpr[16]), CONST(0xf0000000)), CONST(28));
	Value *n = TRUNC1(AND(LSHR(nzcv, CONST(3)), CONST(1)));
	Value *z = TRUNC1(AND(LSHR(nzcv, CONST(2)), CONST(1)));
	Value *c = TRUNC1(AND(LSHR(nzcv, CONST(1)), CONST(1)));
	Value *v = TRUNC1(AND(LSHR(nzcv, CONST(0)), CONST(1)));
	new StoreInst(n, cpu->ptr_N, false, bb);
	new StoreInst(z, cpu->ptr_Z, false, bb);
	new StoreInst(c, cpu->ptr_C, false, bb);
	new StoreInst(v, cpu->ptr_V, false, bb);
}

static void arch_arm_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
		/* Save N Z C V */
	Value *z = SHL(ZEXT32(LOAD(cpu->ptr_Z)), CONST(30));
	Value *n = SHL(ZEXT32(LOAD(cpu->ptr_N)), CONST(31));
	Value *c = SHL(ZEXT32(LOAD(cpu->ptr_C)), CONST(29));
	Value *v = SHL(ZEXT32(LOAD(cpu->ptr_V)), CONST(28));
	Value *nzcv = OR(OR(OR(z, n), c), v);
	Value *cpsr = OR(AND(LOAD(cpu->ptr_gpr[16]), CONST(0xfffffff)), nzcv);
	new StoreInst(cpsr, cpu->ptr_gpr[16], false, bb);
}

static arch_func_t arm_arch_func = {
	arch_arm_init,
	arch_arm_done,
	arch_arm_get_pc,
	arch_arm_emit_decode_reg,
	arch_arm_spill_reg_state,
	arch_arm_tag_instr,
	arch_arm_disasm_instr,
	arch_arm_translate_cond,
	arch_arm_translate_instr,
        arch_arm_translate_loop_helper,
	// idbg support
	arch_arm_get_psr,
	arch_arm_get_reg,
	NULL
};

static uint32_t arm_debug_func(cpu_t* cpu){
	int idx = 0;
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	for (idx = 0;idx < 16; idx ++) {
		LOG_IN_CLR(RED, "R%d:0x%x\t", idx, core->Reg[idx]);
	}
	printf("\n");
#if 0
	if (cpu->icounter > 248306790) {
		if ((core->Reg[15] & 0xf0000000) == 0x50000000) {
			print_func_name(core->Reg[15] + 0x70000000);
		} else
			print_func_name(core->Reg[15]);
	}
#endif
	//printf("run at %x\n", core->Reg[15]);
#if 0
	if (cpu->icounter == 170965) {
		printf("at 790000\n");
		exit(-1);
	}
#endif
#if 0
//	if (cpu->icounter == 687418 || cpu->icounter == 687417 || cpu->icounter == 687416) {
//	if (cpu->icounter >= 0) {
	if (cpu->icounter > 254940700) {
// 	if (cpu->icounter > 1694550) {
//	if (cpu->icounter > 1287900) {
//	if (cpu->icounter > 1696000) {
//	if (cpu->icounter > 779800) {
		printf("icounter is %lld\n", cpu->icounter);
		for (idx = 0;idx < 16; idx ++) {
			LOG_IN_CLR(RED, "R%d:0x%x\t", idx, core->Reg[idx]);
		}
		LOG_IN_CLR(BLUE, "CPSR:0x%x\n", core->Cpsr);
		LOG_IN_CLR(LIGHT_BLUE, "SPSR:0x%x\n", core->Spsr_copy);
		printf("int is %d\n", core->NirqSig);
//		printf("\n");
//		printf("phys base addr is %x\n", cpu->current_page_phys);
//		printf("effec base addr is %x\n", cpu->current_page_effec);
	}
	if (cpu->icounter == 1696000) {
//		exit(1);
	}
	#if 0
	if (core->Reg[15] == 0xc0122570) {
		printf("hit it\n");
	}
	#endif
	#if 0
	if (core->Reg[15] == 0x500083b0) {
		for (idx = 0;idx < 16; idx ++) {
			printf("R%d:0x%x\t", idx, core->Reg[idx]);
		}
		printf("\n");
	}
	#endif
#endif

#if 0
#if DIFF_LOG
#if SAVE_LOG
	fprintf(core->state_log, "PC:0x%x\n", cpu->f.get_pc(cpu, cpu->rf.grf));
	for (idx = 0;idx < 16; idx ++) {
		fprintf(core->state_log, "R%d:0x%x\n", idx, core->Reg[idx]);
	}
#else
	uint32_t val;
	fscanf(core->state_log, "PC:0x%x\n", &val);
        uint32_t pc = cpu->f.get_pc(cpu, cpu->rf.grf);
        if (val != pc) {
                printf("pc is wrong.\n");
                printf("dyncom mode pc is %x\n", pc);
                printf("adu mode is %x\n", val);
		printf("icounter is %x\n", cpu->icounter);
                exit(-1);
        }
	uint32_t dummy;
	bool flags = 0;
	for (idx = 0; idx < 16; idx ++) {
		fscanf(core->state_log, "R%d:0x%x\n", &dummy, &val);
		//printf("R%d:0x%x\n", dummy, val);
		if (dummy == idx) {
			if (core->Reg[idx] != val) {
				printf("dummy is %d R%d : \t[R]%x \t[W]%x\n", dummy, idx, val, core->Reg[idx]);
				flags = 1;
				//core->Reg[idx] = val;
			}
		} else {
			printf("wrong dummy\n");
			exit(-1);
		}
	}
	if (flags) {
		printf("pc is %x\n", pc);
		printf("icounter is %x\n", cpu->icounter);
		flags = 0;
		exit(-1);
	}
#endif
#endif
#endif
#if 1
#if __FOLLOW_MODE__
#if SYNC_WITH_INTERPRET
	if (is_int_in_interpret(cpu)) {
		cpu->check_int_flag = 1;
		return 1;
	}
	return follow_mode(cpu);
#endif
#else
	return 0;
#endif
#endif
	return 0;
}

extern "C" unsigned arm_dyncom_SWI (ARMul_State * state, ARMword number);
extern "C" void arm_dyncom_Abort(ARMul_State * state, ARMword vector);

static void arm_dyncom_syscall(cpu_t* cpu, uint32_t num){

	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	sky_pref_t* pref = get_skyeye_pref();
	printf("in %s user_mode_sim %d", __FUNCTION__, pref->user_mode_sim);
	if(pref->user_mode_sim)
		arm_dyncom_SWI(core, num);
	else
		//ARMul_Abort(core,ARMul_SWIV);
		core->syscallSig = 1;
}

void arm_dyncom_init(arm_core_t* core){
	cpu_t* cpu = cpu_new(0, 0, arm_arch_func);
#if __FOLLOW_MODE__
//	cpu->src_log = fopen("/data/state.log", "r");
	cpu->src_log = fopen("/diff/state.log", "r");
	if (cpu->src_log == NULL) {
		printf("Load log file failed.\n");
		//exit(-1);
	}
	printf("Load source log file successfully.\n");
	cpu->fm_log = fopen("/data/fm.log", "w");
	printf("Create follow mode log file.\n");
#endif

	/* set user mode or not */
	sky_pref_t *pref = get_skyeye_pref();
	if(pref->user_mode_sim)
                cpu->is_user_mode = 1;
        else
                cpu->is_user_mode = 0;
	
	core->NirqSig = HIGH;
	cpu->dyncom_engine->code_entry = 0x80d0;
	if (!pref->user_mode_sim) {
		cpu->dyncom_engine->code_start = 0;
		cpu->dyncom_engine->code_end = 0xffffffff;
	} else {
		cpu->dyncom_engine->code_end = 0x100000;
		cpu->dyncom_engine->code_entry = 0x80d0;
	}

	cpu->switch_mode = arm_switch_mode;
	/* for sync with interpret mode */
	cpu->check_int_flag = 0;
	
	cpu->mem_ops = arm_dyncom_mem_ops;
	//cpu->cpu_data = (conf_object_t*)core;
	cpu->cpu_data = get_conf_obj_by_cast(core, "arm_core_t");
	
	/* init the reg structure */
	cpu->rf.pc = &core->Reg[15];
	/* Under user mode sim, both phys_pc and pc are pointed to Reg 15 */
	if(is_user_mode(cpu))
		cpu->rf.phys_pc = &core->Reg[15];
	else
		cpu->rf.phys_pc = &core->phys_pc;
	cpu->rf.grf = core->Reg;
	//cpu->rf.srf = core->Spsr;
	//cpu->rf.srf = &core->phys_pc;
	cpu->rf.srf = core->Reg_usr;

	
	cpu->debug_func = arm_debug_func;
	
	if(pref->user_mode_sim){
		cpu->syscall_func = arm_dyncom_syscall;
	}
	else
//		cpu->syscall_func = NULL;
		cpu->syscall_func = arm_dyncom_syscall;
	core->dyncom_cpu = get_conf_obj_by_cast(cpu, "cpu_t");
	
#ifdef FAST_MEMORY
	cpu->dyncom_engine->RAM = (uint8_t*)get_dma_addr(0);
#endif
	cpu->dyncom_engine->flags &= ~CPU_FLAG_SWAPMEM;

	if (pref->user_mode_sim)
		core->Reg[13] = 0x0ffffff0; // alex-ykl fix 2011-07-27: need to specify a sp pointer in the correct memory range

	//core->CP15[CP15(CP15_MAIN_ID)] = 0x410FB760;
	core->CP15[CP15(CP15_MAIN_ID)] = 0x7b000;
	//core->CP15[CP15_MAIN_ID + 1] = 0x410FB760;
	//core->CP15[CP15_MAIN_ID - 1] = 0x410FB760;
	core->CP15[CP15_CONTROL - CP15_BASE] = 0x00050078;
//	core->CP15[CP15(CP15_CONTROL)] = 0x00000078;
	core->CP15[CP15(CP15_CACHE_TYPE)] = 0xd172172;
	core->Cpsr = 0xd3;
	core->Mode = SVC32MODE;

//	load_symbol_from_sysmap();
	return;
}

void switch_mode(arm_core_t *core, uint32_t mode)
{
	uint32_t tmp1, tmp2;
	if (core->Mode == mode) {
		//Mode not changed.
		//printf("mode not changed\n");
		return;
	}
	printf("%d --->>> %d\n", core->Mode, mode);
	if (mode != USERBANK) {
		switch (core->Mode) {
		case USER32MODE:
			core->Reg_usr[0] = core->Reg[13];
			core->Reg_usr[1] = core->Reg[14];
			break;
		case IRQ32MODE:
			core->Reg_irq[0] = core->Reg[13];
			core->Reg_irq[1] = core->Reg[14];
			core->Spsr[IRQBANK] = core->Spsr_copy;
			break;
		case SVC32MODE:
			core->Reg_svc[0] = core->Reg[13];
			core->Reg_svc[1] = core->Reg[14];
			core->Spsr[SVCBANK] = core->Spsr_copy;
			break;
		case ABORT32MODE:
			core->Reg_abort[0] = core->Reg[13];
			core->Reg_abort[1] = core->Reg[14];
			core->Spsr[ABORTBANK] = core->Spsr_copy;
			break;
		case UNDEF32MODE:
			core->Reg_undef[0] = core->Reg[13];
			core->Reg_undef[1] = core->Reg[14];
			core->Spsr[UNDEFBANK] = core->Spsr_copy;
			break;
		case FIQ32MODE:
			core->Reg_firq[0] = core->Reg[13];
			core->Reg_firq[1] = core->Reg[14];
			core->Spsr[FIQBANK] = core->Spsr_copy;
			break;

		}

		switch (mode) {
		case USER32MODE:
			core->Reg[13] = core->Reg_usr[0];
			core->Reg[14] = core->Reg_usr[1];
			break;
		case IRQ32MODE:
			core->Reg[13] = core->Reg_irq[0];
			core->Reg[14] = core->Reg_irq[1];
			core->Spsr_copy = core->Spsr[IRQBANK];
			break;
		case SVC32MODE:
			core->Reg[13] = core->Reg_svc[0];
			core->Reg[14] = core->Reg_svc[1];
			core->Spsr_copy = core->Spsr[SVCBANK];
			break;
		case ABORT32MODE:
			core->Reg[13] = core->Reg_abort[0];
			core->Reg[14] = core->Reg_abort[1];
			core->Spsr_copy = core->Spsr[ABORTBANK];
			break;
		case UNDEF32MODE:
			core->Reg[13] = core->Reg_undef[0];
			core->Reg[14] = core->Reg_undef[1];
			core->Spsr_copy = core->Spsr[UNDEFBANK];
			break;
		case FIQ32MODE:
			core->Reg[13] = core->Reg_firq[0];
			core->Reg[14] = core->Reg_firq[1];
			core->Spsr_copy = core->Spsr[FIQBANK];
			break;

		}
		core->Mode = mode;
	} else {
		printf("user mode\n");
		exit(-2);
	}
}

void arm_switch_mode(cpu_t *cpu)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	switch_mode(core, core->Cpsr & 0x1f);
}

static int flush_current_page(cpu_t *cpu){
	//arm_core_t* core = (arm_core_t*)(cpu->cpu_data);
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t effec_pc = *(addr_t*)cpu->rf.pc;
//	printf("effec_pc is %x\n", effec_pc);
//	printf("in %s\n", __FUNCTION__);
	int ret = cpu->mem_ops.effective_to_physical(cpu, effec_pc, (uint32_t*)cpu->rf.phys_pc);
	cpu->current_page_phys = core->phys_pc & 0xfffff000;
	cpu->current_page_effec = core->pc & 0xfffff000;
	return ret;
}

void arm_dyncom_run(cpu_t* cpu){
	//arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	arm_core_t* core = (arm_core_t*)(cpu->cpu_data->obj);
	uint32_t mode;

	addr_t phys_pc;
	if(is_user_mode(cpu)){
		addr_t phys_pc = core->Reg[15];
	}
#if 0
	if(mmu_read_(core, core->pc, PPC_MMU_CODE, &phys_pc) != PPC_MMU_OK){
		/* we donot allow mmu exception in tagging state */
		fprintf(stderr, "In %s, can not translate the pc 0x%x\n", __FUNCTION__, core->pc);
		exit(-1);
	}
#endif

#if 0
	cpu->dyncom_engine->code_start = phys_pc;
        cpu->dyncom_engine->code_end = get_end_of_page(phys_pc);
        cpu->dyncom_engine->code_entry = phys_pc;
#endif

	int rc = cpu_run(cpu);
//	printf("pc %x is not found\n", core->Reg[15]);
	switch (rc) {
	case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
	case JIT_RETURN_TIMEOUT:
                        break;
                case JIT_RETURN_SINGLESTEP:
	case JIT_RETURN_FUNCNOTFOUND:
//			printf("pc %x is not found\n", core->Reg[15]);
//			printf("phys_pc is %x\n", core->phys_pc);
//			printf("out of jit\n");
			if(!is_user_mode(cpu)){
				switch_mode(core, core->Cpsr & 0x1f);
				if (flush_current_page(cpu)) {
					return;
				}
				clear_tag_page(cpu, core->phys_pc);
				cpu_tag(cpu, core->phys_pc);
				cpu->dyncom_engine->cur_tagging_pos ++;
				//cpu_translate(cpu, core->Reg[15]);
				cpu_translate(cpu, core->phys_pc);
			}
			else{
				cpu_tag(cpu, core->Reg[15]);
				cpu->dyncom_engine->cur_tagging_pos ++;
				cpu_translate(cpu, core->Reg[15]);
			}

		 /*
                  *If singlestep,we run it here,otherwise,break.
                  */
                        if (cpu->dyncom_engine->flags_debug & CPU_DEBUG_SINGLESTEP){
                                rc = cpu_run(cpu);
                                if(rc != JIT_RETURN_TRAP)
                                        break;
                        }
                        else
                                break;
	case JIT_RETURN_TRAP:
		if (core->Reg[15] == 0xc00101a0) {
			printf("undef instr\n");
			return;
		}
		if (core->syscallSig) {
			printf("swi inst\n");
			return;
		}
		if (cpu->check_int_flag == 1) {
			cpu->check_int_flag = 0;
			return;
		}
		if (core->abortSig) {
			return;
		}
//		printf("cpu maybe changed mode.\n");
//		printf("pc is %x\n", core->Reg[15]);
		//printf("icounter is %lld\n", cpu->icounter);
		//exit(-1);
		//core->Reg[15] += 4;
		mode = core->Cpsr & 0x1f;
		if (mode != core->Mode) {
			switch_mode(core, mode);
			//exit(-1);
		}
		core->Reg[15] += 4;
		return;
			break;
		default:
                        fprintf(stderr, "unknown return code: %d\n", rc);
			skyeye_exit(-1);
        }// switch (rc)
	return;
}
/**
* @brief Debug function that will be called in every instruction execution, print the cpu state
*
* @param cpu the cpu_t instance
*/

void arm_dyncom_stop(){
}

void arm_dyncom_fini(){
}
