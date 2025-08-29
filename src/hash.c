/* hash.c
 * Performs the zobrist hashing as well as 3fold repetition
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/hash.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

Eval* table = NULL;

const uint64_t zobRandom[781] =
{0xa4eb873de16a53d0, 0xadaba31f919ffb63, 0x3463394ba75e4d58, 0xc2856572e6e47f50,
0x13bd76d905f1559a, 0x689d9826de45d9be, 0x84e1e498ecb9e0a9, 0x82395260744ccfec,
0xe6c5f86665c7b25c, 0x21ee4c2c5e09828, 0x2ea2eea2663ad6df, 0xde598f082b3aaa3f,
0x7310930b996a845b, 0x94f7db84821b3db1, 0x46d4e07f08252500, 0x9391bd639fca9ee1,
0xc7df9f2ba12014d7, 0x4cffdf5a8d5ff31b, 0xf6de1ea70868bda9, 0x80877167114529a2,
0x13f73e8d1e6701c4, 0x4dadfeeff8e4f57d, 0xaade77b30c8f26aa, 0x731039962a1df468,
0x9f31f55a65b2fecc, 0x59bdbafb38278a5d, 0x7251ad6e74783268, 0x8648bcd2faf92d1e,
0x457971a1f53825d9, 0x68d2eda70be31ed7, 0xc52bd4785eabb52f, 0xd951354dfc6b2a2b,
0x540b431a40140618, 0x4b52d883c344f644, 0xdf01ffc8d867a59e, 0x6e473e31cc9c0d8b,
0x4e0c0d4fc45474bb, 0xaaa06c05d55c94cb, 0x727ebb3f95dbae19, 0x4f73b5726ec88dc,
0xc405085b6d06e196, 0x93f2ea11c6eeb856, 0x210e93e772469b93, 0x14fa66f49778802e,
0xbbdabe41afe08804, 0x7eb3460b8bbbc71c, 0xe06049450a1ed285, 0xba7d6021d37153b8,
0x442075370018ee62, 0x3ec74c900dd23107, 0xd3467ee1e755cc1d, 0x3c0f73660810edaf,
0xf727f6fd617a34b7, 0xbf8859cb87fc6cf4, 0x8e62aecf2d694b1, 0xae8f62c85f65515a,
0x2343f194f0a70d93, 0x221411af513c16a5, 0x4bd530fc3024093c, 0x4dad8da529b2d128,
0x4afe5354e01a67d3, 0x7ad55cd0201b9c5b, 0xe0001714521975d9, 0x4be0c3543b02fab9,
0x398086c50969cd47, 0xeb5f2e6375872c25, 0x5d5e712c290a43d1, 0xf87e0ea3ecd84052,
0xfc52bbee55eb1f8e, 0x55e128a4356204f7, 0xb8190bb62482a03e, 0x8a7a2edda741bcdd,
0x61b5a1501d2d18d8, 0xf318d0bf90af1f27, 0x932580dd4824c3a0, 0xaffc4fc35c3f25fd,
0xbef82a6fde4e65bb, 0x4ae28e97b18890f7, 0xd092a1dfacc6c8ee, 0x866017a762b163cf,
0x60d591c525c0caa4, 0xb6c218f211e214e5, 0x980e7d49738ef92a, 0xcb4fa6dc4bb7e486,
0xb1ef75c7c8a60d77, 0xd1b5245b72fc35d2, 0xde27e94ed4be80e0, 0xac520b8851bb2822,
0xed3b37d3b76ab101, 0x16ad1ccab79adf15, 0x1c27ef85bdaa2d0c, 0xddbe58d95bb5d48d,
0x2e5a7b9cb4be7d3e, 0xad27433a8fb764b3, 0xed46803e8a40208, 0x4eec715fdd0c7489,
0x29a2741f45adf6b2, 0xdf719da7be2fafb3, 0xa0e33fa334033e5, 0x711184a200daf53f,
0x8f7073810427bd75, 0xfe965af25cd5c240, 0x3a22205f582d421, 0x9cb3cf01cc7620e6,
0x2e0aba88502ded01, 0x254df8d7a05ad133, 0x1d05f896d75c829d, 0x78247a792054a985,
0xe88bc42a5f6ba4eb, 0x1bae2194ddbf33b0, 0x6e9f1dabba7593cd, 0x335d9a9c74462011,
0xdc01f27d1b987bc5, 0x4ebfc73cc88b3030, 0x47f4cded5886bd08, 0xae99f6f8350f89c3,
0x87369d0144bcc968, 0xd8f0c65e47385b0f, 0x8799a1e346beb0b0, 0x7856f386ae307c56,
0x94b4302bbb61cd2d, 0x4471cbe0877e72fc, 0xdcd479f3f3cd0427, 0x5e76ca4a75dd22a8,
0x3a203739920fd588, 0xcaf21d69e2dacef3, 0x73f07f781afc1e95, 0x9c9aca4a500c5f3d,
0x9003fbe105bf3e2, 0xd3a1cbc4ef338a1d, 0x8315a8b1a4ccf95, 0x36b048e0093d00d9,
0x19cd603d0cea7555, 0x783db976ae79071a, 0xe3cbdff7dca867f5, 0x3768811b81cdd4b7,
0xeaa6d78c8c4880a2, 0x970165f22fc7d817, 0x230f744885a49213, 0xae70cf6d607d8bc0,
0xdda7b22a12855679, 0x929f70d239ce760b, 0xd57dba5e84a976f1, 0xabc9037398ed4605,
0xa8ed72ef5308b96b, 0x2ff0da79fc11373e, 0x604f22611dd70334, 0x1a4aa8fd869277eb,
0xe85fdb11c590a07, 0xb81877f0b165398d, 0xfab7b97814670b10, 0x5f4f9d0fcce8e6b1,
0xc72af8139f567fed, 0x857464899ae4f392, 0x38b34d5e37b9d350, 0x4263e944798c1a94,
0xc485f26811abd527, 0xe6980fccb26735ad, 0x1ae83b495ad58130, 0x77386accd32aed3,
0x7f7dec526d26e172, 0x9903cf37b4cc0588, 0x94ffe61eb1908c71, 0x5113c8efdce2f576,
0x70cdbf35b3f183cf, 0x9f40975571125386, 0xe8a5e12e12cf9ff8, 0x3cd4df683d7ef5bd,
0x862e66b2238e7ed2, 0x89781281e708c41e, 0xde66370ee76aa0a6, 0x651de31e7aedb370,
0x33ee34175e0116ca, 0xcaa63e3ec7611266, 0x9552a60109786124, 0xfee5108f9e16ad9a,
0xbed8e5c72ab3bb3a, 0x5c95780b6364af83, 0xeb8f5e5ba9d591ba, 0x62724bf340fc8fd4,
0x868a18dd1c517959, 0x6a9ec68cc82dc96d, 0x3fdfb8a40fc802c3, 0x46ab8623b0b0f5f0,
0x185312fa044ce079, 0x2480712b3fb7e5a, 0xa5fcf8248d2fef56, 0x1c3e6bdfd105a5e2,
0xb6d07770ea45d4d5, 0xef93dd034e001d7f, 0xb95a89b3a77bdbe7, 0xd16e22c77d3b3af6,
0xeaf5091c25a40a02, 0xcc7b06896cb562a6, 0xdf62545c68ee195f, 0xa5928ce3948c6c24,
0x40e58e527d7b9c0, 0x641b8f74a37045ad, 0x97fb09c36187c5ea, 0x496bf220646c6e7d,
0xf6f91894c18b5b27, 0xf6cd098865a6f8f4, 0x1ff2e0fb7bc83056, 0xfe8dec30e64a0b86,
0x1b3224ac3fe886cf, 0x5b9ff141f7e3ced2, 0xa538be1a068366af, 0x6f311b8df5c9c209,
0x4aa6bfaddc3d79b6, 0xcec7cab5126a1374, 0xddac9425e4e40284, 0x120137bc753ae611,
0x8af5a1835048849b, 0xf68c04d5f2c1dea1, 0xd770b74b9b74576a, 0x6d2213fb5383986d,
0xa996e46e99b9db7, 0xcf822bafabc0a2c7, 0x30618771f0f82193, 0x7e9209c563515095,
0x497b326c8798de33, 0x2e6008010b17c2af, 0xf10faddd5d08ec76, 0xad8bdc4dd6cab54d,
0xe042548ae68cba17, 0x64e8b5ea2004ca6d, 0xd7f24b8751ee1abc, 0xd77eb64071363a07,
0x88f6d45be44d2dda, 0x91ac0d61301b5304, 0xe86e33beb5aa507c, 0xf34454f7b0f58585,
0xde72804e05e29390, 0x85231751382f5ace, 0x1f9e3f6130344487, 0xb6863b388a9b0152,
0xa37307fb3ac872a9, 0x74b0d92ea9a72791, 0x713f74601418af07, 0xf885ff501de51396,
0xce944143fe0c27f7, 0x399e65b17ff709e5, 0x455698ab34a0e9d5, 0xd8ac02cf66c81ec5,
0x8e8c19621a49285f, 0x4d8e84452603314c, 0x4a7e793c71027ea6, 0x6f7d9c4a96b91d2b,
0x907adb70ae35520, 0x91e68adaf16c1c63, 0xc2c874f7adb21c39, 0x345910a8c8f9b1d3,
0x903392e852d2f759, 0x884577c7616a29d6, 0xdf026199050f5446, 0xc86d7805f4f6f47e,
0x5edcb283ff791ac4, 0x862c409e6d081dc, 0x7580ca55845f80e6, 0x2ea2ff4af379f1e,
0x520ea3398aa5a71f, 0xe4d3e53024bc6633, 0x572d523956013489, 0x2434db4fa23e69b0,
0x669cbef6457c8a92, 0xd50a6df78a6f274f, 0x7622effe2743b110, 0x8275adc24a866a92,
0xf29761fbad35d22a, 0x2cd14bf411472735, 0x707d5e00edf030d, 0xe0ff87c9f9c94fa6,
0x32637a441a09f60f, 0xa0f5563c02d9c378, 0x20ef1e0de8a34e31, 0x2c87ed54ce735c94,
0xba8d41937517c561, 0x51d64c955d5a6fb8, 0xa69c337987058d83, 0x99bbd326614f37d4,
0x3a7309a7efd98509, 0x408d11e159bf4374, 0xec7043d4eb2a190a, 0xa86aeb4b3e6a91eb,
0xbe77a1849f5e6fa3, 0x8ee09d456af5d3e4, 0x8eaf685217f4b84e, 0x7d5184100d2eb4a4,
0x94258458c7f12b52, 0x6575d7192bce2c7b, 0x3897e07f75b97cf5, 0x282e477012833da1,
0x7d37738a049d8568, 0x4ced430c6f7bd022, 0x42d2c5d3251e426a, 0x5f9fe26bc3ffc216,
0x4a2a60730d5c5352, 0x23f78956793074af, 0xdbd308c6c87048ef, 0x88f938f96ec5197f,
0x31e777b38149c852, 0x1de79bb5293c933d, 0xa0acb89cc53552a3, 0x1e50278053a34704,
0x4d0700d5fb1c816d, 0x3b1d547e43a27f39, 0x2d11ec1bee420a3e, 0x2193d5f1d01156b1,
0x819c69365a0efef9, 0x907152934fd36c30, 0xe687645fe30273f9, 0xc91bc1a99e26a59c,
0xa8cc05df5abe596e, 0x8596222cea1d064f, 0x81000e63e8011b84, 0xb9fba20e4b10008f,
0x3f4f84ddc981a798, 0xadd710b2c2edb672, 0x80c5a301119d3660, 0x6d290df4af13da43,
0xb64a61b1b0de7e7f, 0xcafbbfc688a7e63, 0xd7072cdd68efba3, 0x715e602c538c8ef2,
0x24fb6c8cc800cd48, 0xb2af0d078516c83f, 0x49e8e6ce5e0f98bf, 0x1230a8a2f55d9906,
0x6d99416550895b4c, 0x49a4e053133fbc46, 0xf1056c4adc26ac10, 0xe0a0d8f26891f87b,
0xfd6c90add7cd9517, 0x32d5d100ffd9dea4, 0x27af7b03b8fa8b8b, 0x34c50a8d89ab4242,
0x5d93a229ab81c4c3, 0x88f71cc7eb81b8b6, 0x364607ff2a18f893, 0xe303f06e1d434050,
0x725856c5b8577b9d, 0x693b1a288ce6ef6b, 0x389c8dc77bb9af21, 0xcec96a75a9020890,
0x26146d0c3db29f35, 0x8819d07002c082b2, 0xeedc59d8dc12b0ad, 0xac4ae13fca649361,
0x8ca1e2f99313f75d, 0xbb148321670e6686, 0x4e0873a53fbf3075, 0x5e48a3da5c6442ed,
0x302f661e08d340c1, 0x153adc805955d61f, 0x72e6a9ca62d10224, 0x532669250985381a,
0x58bb4a4c298fb347, 0x4cdf2e82673f8896, 0xbba6d99a8d3b195e, 0x16073fb91611fe75,
0x8a241d7bd03b1ffe, 0x5d402b45e798ae92, 0xc14ef59e1477eab5, 0x62c66fd1b655e98a,
0x4b8c1b9d9d478a97, 0x76b0c3d9cf9bdaa0, 0x24c524c49ae2e83f, 0xfd002005ec82b845,
0x548baa26ae752f50, 0x24fec02eabfe41d8, 0xce0c22a7d784d310, 0x657c8af8534a5dc9,
0xfcb5a46a9f95a2be, 0xfd70418dc37a35ec, 0x5c4d9fa7edd31ea6, 0xd0457ba9da458662,
0x469ac214197cb2a4, 0x934a00434da81758, 0x7128b60388d8c611, 0x3019a9e2241b17b,
0xe1e0292361512cd2, 0x96d1742b2fda766c, 0xde9f5afe71a7a0f0, 0x81443d7cb7dbcaf4,
0x4d74467ad220a920, 0xe5eb92b89eae56bc, 0x7b3edb4e3f4be5fe, 0xb96b3a78f76a178f,
0x111cec3cc8e35dcc, 0xdf38793d7ad1af69, 0x3249de10b764fade, 0xf0c58cd02640c570,
0x8bdbd4c09bab9d0b, 0x325e221e65da2310, 0x17d1bb792b96f583, 0xaaf4a0163e65469d,
0xd1c9eb52b1e7fd21, 0xa58261e05e3453a7, 0xb26e2aee187c4f75, 0xfbbefce23284894b,
0x47cbef6f46b40115, 0xf25da739868ca56a, 0x54df10b8883b7e62, 0xdba1ff0048c73947,
0xa13dfd23829d8a90, 0xed6bb014afb647f5, 0x8a57b412464038a7, 0x947cdeef45db6987,
0x3290b6e189ebdc8f, 0xb5d87c63f8623cf5, 0x8d30dc1fbb30b4f2, 0x84cffd9bf24f0a24,
0x183d3e58aa450c8f, 0xf82c363c1884354f, 0x47281dddf6d6d05d, 0x57252567c37670c1,
0xbe4f5d2f933b9ae4, 0x794343e0442406a4, 0x8ad5af8c62ee49e2, 0x8cc1eef36b80a492,
0x5c4007c089fd0c99, 0xaf22e0dd969ae543, 0x66576af3c65dc859, 0xeed2cce1909204c4,
0x4765f877d507bba1, 0x63cdc967961e3e37, 0x5361d92fd4aba277, 0x36c3325b3698c269,
0xba71b4abb38b0b95, 0x8c6b5d25d67b3e39, 0xad1b5165071ada63, 0xa8376d2336491f5c,
0x8d52b61250e3b76f, 0xe84c398fe903656c, 0x78a78334f47a3aa8, 0x76cd477134323619,
0x52bd701c91b735fd, 0xd1149266f574feba, 0xd45f23a35192027e, 0x96fed667a6f5a9ba,
0x16e01d8aa9507532, 0x1a20573cc818a230, 0x60a157d4fa3bd686, 0x653f1309279429b0,
0x5c570c44b031ab90, 0x7b5aa948c3f056f3, 0x3afdf1aa409696ba, 0x336de07df0f9e55f,
0x80f152db333db3f8, 0x90819c6a83d9014f, 0x2c0a4a8dfbcfe27, 0xd340e3d4bff73be5,
0xd8d31a069cc61bdb, 0x77172073b3208a6, 0x605386b3f895a6f4, 0xdba148f7a49d1f0,
0xe661ed28323d0555, 0xa1593463e9640a3e, 0x1595ed5c7fadd7f8, 0x84d552802d5c98d5,
0xe3f9b7d950671e71, 0x582bba23276775d4, 0x13a4955e807edf47, 0x875be98643104ae0,
0x2c20b53f34db416c, 0x1a5f7ed1b3e68600, 0x5e816cf6753eed01, 0xdf7e9ebb84325b88,
0xd501ffb5abc10701, 0x5ef32e4961204572, 0x57eeeb1eccf2aac3, 0xa55443e37dbffd4b,
0xc49611c9093e6e17, 0xb25bb1f2dd41598d, 0xaa7bfd81fecd0f71, 0x98d2a139f675bd93,
0xa75f6f09fead6c1a, 0x7f39608bb166917e, 0xcc6969003b03aebe, 0xdc1ef4ef4023faac,
0x49e81853ce767526, 0x336bb9d414c916fc, 0x4a8d413060d28bc6, 0x56beec06e52333b3,
0x53d6a269fee4af5d, 0x88990711153e02fb, 0x463c449e9215b6b1, 0xea875a5ef542a76b,
0x44a2551c3bc23d8d, 0x38dc888ab448e464, 0xa25dd6250dfc7701, 0xaf83cbad2aa519ec,
0x9b39066d6afb816c, 0x57a68f40a53a76, 0xe7958e296a8f7fac, 0xc10477091ce09a58,
0x5440091020670cba, 0x674a27ada2ecefa4, 0xe15ddcd7eeaf8839, 0x299aac0bd135c62f,
0xe49825162459bd97, 0xb0217c444966dec6, 0xd19425cd40c97219, 0xdcdece88c39bbe25,
0x724d07f1ce046659, 0x5985a6151d1b4874, 0x8f5d2bf13a44f107, 0x73da5e8459ce366f,
0x7256bbd921ed7a46, 0xc5e0c11f592f2da9, 0xb11da8eaa1935e9a, 0x798eeb783b428950,
0xf63088e8f16a6f51, 0xfcce25d320c92c80, 0xec7511d5e15ddb90, 0x19d72fbc7dd6318e,
0xf6e69f2bc97af6bd, 0x2b90b68d45e35b62, 0x5179436c096c44f1, 0x825c20a2e9f547da,
0xaf4b947dbda5750, 0xea26a989c45dda89, 0x250f1ca030816526, 0xb0f3e24d23e2a5d3,
0xe1b80e8428f402b4, 0x1102b1d9a5928e4a, 0x55eaa91de58166d1, 0x9765c7e173e19da0,
0x13088ee11723d513, 0xf70aed4770c6cad3, 0x405a9bce64a171e2, 0x7991622d5bea3bd5,
0xbb02dd1675fba937, 0xab729c420054e998, 0x776595af03356b0, 0x894d926189a63572,
0xeb7b58c586890919, 0x345c28b7658f1a18, 0x9ea6fd6b0228b50c, 0xda6082a6bd156bc4,
0xc27340d8eb51f9be, 0x8777a929401bfc29, 0xc6ce2f0565648e28, 0xcc011803a73225a8,
0x13eaffdf000590c5, 0x8b3f33db432eff56, 0x92206a882ab0660e, 0xf0c0125db75d1530,
0xcf639988608cb5d7, 0xabe3760ba041613a, 0x819657948cb1d858, 0x552e3127a810e8b0,
0xf307cfefb47f26e9, 0xd30799a6205dabe0, 0x2c1e6e6b3685cc93, 0x4f55169e6df79851,
0xa824220d39b33731, 0x2e94159cd7a0c359, 0xd183f82309837160, 0x2ef76d59943e66ce,
0xc732098e9d7d7cef, 0x56916c85af2e03dc, 0xc3ea07e0fa2dac03, 0x89ecd91fe1e5e8ea,
0x3c2fec44fdb2302, 0xfb730a3c80a3e68, 0x9bf65900d4e0d11, 0xd03bb57a73800e46,
0x5e24147906c74710, 0x75881dbd1e9cecab, 0xea28d87d3781cc7a, 0x88f1d6a5b6ee1aca,
0x3739b66edbee374e, 0x8f4675f55912e60b, 0xa44ced560c6462f6, 0x19be480e8235c7cc,
0x6b142044292670eb, 0xf093d4b9c4b14a3a, 0x3d9ece2c5ccd546f, 0x3a4f4f17122f033c,
0x2032e4bdcfa6ce62, 0x366b1edd0e8720e1, 0x89cdadadd36f2f3e, 0xbbf402c702a1b85c,
0xd57cac15ac181aef, 0xfeae32475fd317de, 0xbb53c87828148e09, 0xa709baf9497b87f,
0x2bc0feee8048cb78, 0x66abd6679ad871a8, 0xe722db76a3802996, 0xb4be6907896f461e,
0x56bfc42a4f69a9af, 0x1cf3390b6b4c5d4c, 0xe13be9b463797eb1, 0xa8820d025c75ce03,
0xb1d6d2c3ae8e905, 0xa0bb677a6f1bf3b6, 0xfc7f95851a7a92e6, 0xf85dde67d7e469a1,
0xa7720b93943f908f, 0x2b727873a8e1ff60, 0x9b6997fb13534af4, 0x8b467f29340dc896,
0x35f5ef926d286b85, 0xc3f3f82b52f03e2b, 0xb038c62931222794, 0x7b51aba6f8c99235,
0x79b8e685ce89507f, 0xa2077f95c5ecc32d, 0x714f9b6298e8823c, 0x6fc03cd370e8b56,
0x9f7a1a7d97188e04, 0xcd21dc2e2e35dd4b, 0xafb1372369be108f, 0x90b73ed3968aa6b0,
0xc8ebaad6276f70f4, 0x8ebfc8fdec3719d3, 0x67956523cea43ade, 0xed275b837d307013,
0x75e106641f9d4fd3, 0x5d7334064a1dc727, 0x87d10109dc1bd3cb, 0x130d36f2d921f1c4,
0x887b4c536c81e04f, 0xf7590691dec5f2c3, 0x8b27c76ece2ecde0, 0x4e304b4328a1829e,
0x7d4eadb9ec511555, 0x33ef8850030aad57, 0xb7c69e7951f6e851, 0x492f4c624e1102df,
0x85224e0631b1a02a, 0x3451610c0598a834, 0x8391979b9c77e679, 0x198b74f79289d7b3,
0x398d684a59cbb018, 0x729c8ab5cbb26812, 0x9fc10f0df5117bf5, 0xfa69a71b93f6d16a,
0xe30839d9744a14ed, 0xfdb0701c28436bde, 0xcf26d53d3cc52844, 0x8e86297421898150,
0xe27e8196a817439f, 0x81460e8834b3d662, 0x118fb5e67151f0df, 0xf1b9ce6c53473b87,
0xce65fddbc247bfc7, 0xc4850e3f2ed17882, 0xbfa1be5690959c9d, 0x5f39db1cadd2bc4f,
0x198693efa729fa8, 0xe0b1346c810ca25f, 0xffb0dda172458081, 0x3ee90e6055dea86c,
0xf275637809766957, 0xf5af186a8ddbbeb2, 0x4a07259ed41e7649, 0x6b3ba54daf1167ab,
0x9708f6532e7a35ba, 0x549720ff1e4c6763, 0x60c7eb0dfc1a76db, 0x5c3daf52bb167d42,
0x7aeb10d5261af612, 0x89a2e1b335e5b6b2, 0xb261225b382f725a, 0x7a1f37db99425e90,
0xad9db5318db29fcf, 0x60c934ef4390e913, 0x83b78bc4f0e57ab7, 0x66c8c56c890af035,
0x969feaeaab815de7, 0x21721c102b1a3c8b, 0xf3425e450cffc770, 0xbcd6b2358dd36b06,
0x1a2a51846b373f23, 0xa8029e5bb82ae08d, 0xb25db53cd7dd9ef8, 0x72678ff8216d20dc,
0x42a567e977679c60, 0x6085320b057d5c8c, 0x1bc3e9a51a1bcb19, 0x6a658858954b1fab,
0xca929b21dbc435ea, 0x632541d8b0e3bf40, 0xa11ffdeacb6f90c4, 0x5fe1813da2aa5630,
0x310d8c686e239e0e, 0xb991c74685c3eea5, 0xed9e2d81eaaadc83, 0xffd926173123f72b,
0xe8b8c8318aa52ff0, 0x767e86f10276ab4b, 0xea08f1df5d95a447, 0xc99263da0c863911,
0x5235361585ea82d2, 0x6f8ba1001b5dbf72, 0xbc2a6000cc58a763, 0xb0f849dd35a7d1f,
0x5a55adbcb1bde46b, 0xd0b1985b58cb94d, 0x9f02ddc8926ac31c, 0x9c86105630d68f33,
0x1c47aef1caee8cf5, 0xccdc72ef5729c132, 0x140110ab78263d84, 0x4187d53d13346f53,
0xefc453b41883a5e5, 0x1451142d0428ac39, 0x40a484fcabac832f, 0xb5131819a9ea1dfc,
0x204bd53976c6bd6d, 0x6f9fdec1df29eaf2, 0xb893e39430003905, 0x4f127d5b38945fcb,
0x7be9d44ab6c446da, 0xb0506ba21c5e8f91, 0x1583b1a0e7f70e75, 0xc559cba411b4bf59,
0xc4d76871f19361e3, 0x6289bb800c4cb025, 0x98db29f1402e3b4a, 0xe5c5db6aeb7fee75,
0x5b14a47576a6c177, 0xaf05a5f86b1cfbe5, 0x44d17f542d9d3025, 0xf0b3660e976470cb,
0xc184630e6b2fc09, 0xafe872fef75fd02c, 0x7881f5566256e5c2, 0xd0370f8ce09962d3,
0x99d069864300da4f, 0x13c4cedc00018d87, 0x3b2a53959b627d35, 0x7bab6948e02441ab,
0xe0773e5335bbe9c9, 0x788b787a3bcce1f7, 0xf1fe51e9cac3d588, 0x35b2391d4af71618,
0xbb6e6a4c27e94555, 0x422999537afc3220, 0x9fff33082c23aa7, 0xbb57415bc9499d18,
0xb599feed164367f0, 0xb9ffc7b520351019, 0x873567cc10562a8a, 0x1a0c5e97dbce42fb,
0xbe7421b4e03d187d, 0x604521a34002cbf, 0x7dfd18d2e4e08e85, 0x1e69e98589fb20ea,
0xbc202e5b88530001, 0xd899677f4705e76d, 0xeb18d5721002d9ff, 0xa93229a83f4b4ed1,
0xff1acc4e19be5dc1, 0x6b43b38eeeed5176, 0xb2ca2d96b86a2bb8, 0x922e54f31a04c813,
0xb38760e7ed480e0c, 0x958ef021026a6a28, 0xeda0af852feb872a, 0xf3528a827b308642,
0x3c5e3024409e883c, 0x5dc4606c234372e4, 0x9f58ce6c5342825b, 0xfc5f81990a4b7f1b,
0xae240a5da5373b0, 0x4ed234968bdcede6, 0x9cf317433a248cf0, 0xe888d6ed05910af3,
0x95e465db60d6e7e2, 0xa751158208d87aec, 0xb83c21445097d153, 0xf74566556bcf68d,
0x89e921a9a45eeb6, 0x613102a85c0bab6c, 0x79e85d0669719a02, 0x431823972da34798,
0xeba616ad483651dc};

/* Ensures that all the keys are 0
 */
void initializeTable(void)
{
    free(table);
    table = calloc(NUM_ENTRIES, sizeof(Eval));
    CHECK_MALLOC(table);
    /*
    Eval* end;
    end = table + NUM_ENTRIES;
    for (Eval* ptr = table; ptr < end; ++ptr)
        ptr->key = 0;
    */
}

void reinitializeTable(void)
{
    memset(table, 0, NUM_ENTRIES*sizeof(Eval));
}

/* Detects if the last move makes a 3fold repetion
 * PRE: Hash isn't in the array
 */
int isThreeRep(const Repetition* r, const uint64_t hash)
{
    assert(r->index >= 0);
    int count = 0;
    for (int i = r->index - 2; i >= 0; i -= 2)
    {
        if (r->hashTable[i] == hash)
            ++count;
    }

    return count > 1;
}

uint64_t calcPos(const int color, const int piece, const int sqr)
{
    assert(KING <= piece && piece <= PAWN);
    assert(color == BLACK || color == WHITE);
    return zobRandom[(color * COLOR_OFFSET) + (piece * PIECE_OFFSET) + sqr];
}

/* Hashes a position
 */
uint64_t hashPosition(const Board* b)
{
    //Turn
    uint64_t resultHash = (uint64_t)b->stm * zobRandom[TURN_OFFSET];
    //EnPass
    if (b->enPass) resultHash ^= zobRandom[EPAS_OFFSET + (b->enPass & 7)];
    //Castling
    for (int i = 0; i < 4; ++i)
    {
        if (b->castleInfo & (1 << i))
            resultHash ^= zobRandom[CAST_OFFSET + i];
    }

    uint64_t temp;
    for (int i = BLACK; i <= WHITE; ++i)
    {
        for (int j = KING; j <= PAWN; ++j)
        {
            temp = b->piece[i][j];
            while(temp)
            {
                resultHash ^= calcPos(i, j, LSB_INDEX(temp));
                REMOVE_LSB(temp);
            }
        }
    }

    return resultHash;
}

inline uint64_t changeTurn(const uint64_t prev)
{
    return prev ^ zobRandom[TURN_OFFSET];
}
/* Updates the hash of the position, prev is the hash of the position before the move
 * PRE: The function is called after the move is made
 */
uint64_t makeMoveHash(uint64_t prev, Board* b, const Move m, const History h)
{
    prev ^= zobRandom[TURN_OFFSET];
    const int col = b->stm;
    const int opp = 1 ^ col;

    //Piece from
    prev ^= calcPos(opp, m.piece, m.from);

    int xorCastle;
    //There has been a capture
    if (m.capture > 0)
        prev ^= calcPos(col, m.capture, m.to);

    if (h.enPass)
        prev ^= zobRandom[EPAS_OFFSET + (h.enPass & 7)];

    switch (m.piece)
    {
        case PAWN:
            if (m.promotion > 0)
                prev ^= calcPos(opp, m.promotion, m.to);
            else
                prev ^= calcPos(opp, PAWN, m.to);

            if (b->enPass)
                prev ^= zobRandom[EPAS_OFFSET + (b->enPass & 7)];

            if (m.enPass)
                prev ^= calcPos(col, PAWN, m.enPass);
        break;

        case KING:
            if (m.castle & 1) //Kingside
            {
                prev ^= calcPos(opp, ROOK, m.to - 1);
                prev ^= calcPos(opp, ROOK, m.to + 1);
            }
            else if (m.castle & 2) //Queenside
            {
                prev ^= calcPos(opp, ROOK, m.to - 1);
                prev ^= calcPos(opp, ROOK, m.to + 2);
            }
        /* no break */
        case ROOK:
            if ((xorCastle = b->castleInfo ^ h.castleInfo))
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (xorCastle & (1 << i))
                        prev ^= zobRandom[CAST_OFFSET + i];
                }
            }
        /* no break */
        default:
            prev ^= calcPos(opp, m.piece, m.to);
        break;
    }

    return prev;
}