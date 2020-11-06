#include "fast_float/fast_float.h"
#include <iomanip>

inline void Assert(bool Assertion) {
  if (!Assertion)
    throw std::runtime_error("bug");
}

template <typename T> std::string to_string(T d) {
  std::string s(64, '\0');
  auto written = std::snprintf(&s[0], s.size(), "%.*e",
                               std::numeric_limits<T>::max_digits10 - 1, d);
  s.resize(written);
  return s;
}

template <typename T> std::string to_long_string(T d) {
  std::string s(4096, '\0');
  auto written = std::snprintf(&s[0], s.size(), "%.*e",
                               std::numeric_limits<T>::max_digits10 * 10, d);
  s.resize(written);
  return s;
}

bool basic_test_32bit(std::string vals) {
  std::cout << " parsing "  << vals << std::endl;
  float result_value;
  auto result = fast_float::from_chars(vals.data(), vals.data() + vals.size(),
                                      result_value);
  if (result.ec != std::errc()) {
    std::cerr << " I could not parse " << vals << std::endl;
    return false;
  }

  std::cout << std::hexfloat << result_value << std::endl;
  std::cout << std::dec;
  return true;
}

bool basic_test_32bit(std::string vals, float val) {
  std::cout << " parsing "  << vals << std::endl;
  float result_value;
  auto result = fast_float::from_chars(vals.data(), vals.data() + vals.size(),
                                      result_value);
  if (result.ec != std::errc()) {
    std::cerr << " I could not parse " << vals << std::endl;
    return false;
  }
  if(copysign(1,result_value) != copysign(1,val)) {
    std::cerr << vals << std::endl;
    std::cerr << "I got " << std::hexfloat << result_value << " but I was expecting " << val
              << std::endl;
    return false;
  } else if (std::isnan(val)) {
    if (!std::isnan(result_value)) {
      std::cerr << vals << std::endl;
      std::cerr << "not nan" << result_value << std::endl;
      return false;
    }
  } else if (result_value != val) {
    std::cerr << vals << std::endl;
    std::cerr << "I got " << std::hexfloat << result_value << " but I was expecting " << val
              << std::endl;
    std::cerr << std::dec;
    uint32_t word;
    memcpy(&word, &result_value, sizeof(word));
    std::cout << "got mantissa = " << (word & ((1<<23)-1)) << std::endl;
    memcpy(&word, &val, sizeof(word));
    std::cout << "wanted mantissa = " << (word & ((1<<23)-1)) << std::endl;
    std::cerr << "string: " << vals << std::endl;
    return false;
  }
  std::cout << std::hexfloat  << result_value << " == " << val << std::endl;
  std::cout << std::dec;
  return true;
}

bool basic_test_32bit(float val) {
  std::string long_vals = to_long_string(val);
  std::string vals = to_string(val);
  return basic_test_32bit(long_vals, val) && basic_test_32bit(vals, val);
}

bool basic_test_64bit(std::string vals, double val) {
  std::cout << " parsing "  << vals << std::endl;
  double result_value;
  auto result = fast_float::from_chars(vals.data(), vals.data() + vals.size(),
                                      result_value);
  if (result.ec != std::errc()) {
    std::cerr << " I could not parse " << vals << std::endl;
    return false;
  }
  if(copysign(1,result_value) != copysign(1,val)) {
    std::cerr << "I got " << std::hexfloat << result_value << " but I was expecting " << val
              << std::endl;
    return false;
  } else if (std::isnan(val)) {
    if (!std::isnan(result_value)) {
      std::cerr << vals << std::endl;
      std::cerr << "not nan" << result_value << std::endl;
      return false;
    }
  } else if (result_value != val) {
    std::cerr << vals << std::endl;
    std::cerr << "I got " << std::hexfloat << result_value << " but I was expecting " << val
              << std::endl;
    std::cerr << std::dec;
    std::cerr << "string: " << vals << std::endl;
    return false;
  }
  std::cout << std::hexfloat << result_value << " == " << val << std::endl;
  return true;
}
bool basiciss_test_64bit(double val) {
  std::string long_vals = to_long_string(val);
  std::string vals = to_string(val);
  return basic_test_64bit(long_vals, val) && basic_test_64bit(vals, val);
}

bool issue8() {
  const char* s =
      "3."
      "141592653589793238462643383279502884197169399375105820974944592307816406"
      "286208998628034825342117067982148086513282306647093844609550582231725359"
      "408128481117450284102701938521105559644622948954930381964428810975665933"
      "446128475648233786783165271201909145648566923460348610454326648213393607"
      "260249141273724587006606315588174881520920962829254091715364367892590360"
      "011330530548820466521384146951941511609433057270365759591953092186117381"
      "932611793105118548074462379962749567351885752724891227938183011949129833"
      "673362440656643086021394946395224737190702179860943702770539217176293176"
      "752384674818467669405132000568127145263560827785771342757789609173637178"
      "721468440901224953430146549585371050792279689258923542019956112129021960"
      "864034418159813629774771309960518707211349999998372978";
  for (int i = 0; i < 16; i++) {
    // Parse all but the last i chars. We should still get 3.141ish.
    double d = 0.0;
    auto answer = fast_float::from_chars(s, s + strlen(s) - i, d);
    if(answer.ec != std::errc()) { std::cerr << "parsing failure\n"; return false; }
    if(d != 0x1.921fb54442d18p+1) {
      printf("%.*s\n", int(strlen(s) - i), s);
      std::cout << std::hexfloat << d << std::endl;
      std::cout << std::defaultfloat << d << std::endl;
      return false;
    }

  }
  return true;
}

bool check_behavior() {
    const std::string input =  "abc";
    double result;
    auto answer = fast_float::from_chars(input.data(), input.data()+input.size(), result);
    if(answer.ec != std::errc()) {
      std::cerr << "parsing failure as expected\n";
      // specification says ptr should point at first
      if(answer.ptr != input.data()) {
        std::cerr << "If there is no pattern match, we should have ptr equals first\n";
        return false;
      }
      return true;
    }
    std::cout << "parsed the number " << result << std::endl;
    return false;
}

bool issue19() {
    const std::string input =  "3.14e";
    double result;
    auto answer = fast_float::from_chars(input.data(), input.data()+input.size(), result);
    if(answer.ec != std::errc()) {
      std::cerr << "We want to parse up to 3.14\n";
      return false;
    }
    std::cout << "parsed the number " << result << std::endl;
    if(answer.ptr != input.data() + 4) {
      std::cout << "Parsed the number and stopped at the right character." << result << std::endl;
      return true;
    }
    return false;
}

int main() {
  Assert(issue19());
  Assert(check_behavior());
  std::cout << "======= 64 bits " << std::endl;
  Assert(basic_test_64bit("INF",std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-INF",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("INFINITY",std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-INFINITY",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("infinity",std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-infinity",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("inf",std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-inf",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("9355950000000000000.00000000000000000000000000000000001844674407370955161600000184467440737095516161844674407370955161407370955161618446744073709551616000184467440737095516166000001844674407370955161618446744073709551614073709551616184467440737095516160001844674407370955161601844674407370955674451616184467440737095516140737095516161844674407370955161600018446744073709551616018446744073709551611616000184467440737095001844674407370955161600184467440737095516160018446744073709551168164467440737095516160001844073709551616018446744073709551616184467440737095516160001844674407536910751601611616000184467440737095001844674407370955161600184467440737095516160018446744073709551616184467440737095516160001844955161618446744073709551616000184467440753691075160018446744073709",0x1.03ae05e8fca1cp+63));
  Assert(basic_test_64bit("-0",-0.0));
  Assert(basic_test_64bit("2.22507385850720212418870147920222032907240528279439037814303133837435107319244194686754406432563881851382188218502438069999947733013005649884107791928741341929297200970481951993067993290969042784064731682041565926728632933630474670123316852983422152744517260835859654566319282835244787787799894310779783833699159288594555213714181128458251145584319223079897504395086859412457230891738946169368372321191373658977977723286698840356390251044443035457396733706583981055420456693824658413747607155981176573877626747665912387199931904006317334709003012790188175203447190250028061277777916798391090578584006464715943810511489154282775041174682194133952466682503431306181587829379004205392375072083366693241580002758391118854188641513168478436313080237596295773983001708984375e-308", 0x1.0000000000002p-1022));
  Assert(basic_test_64bit("1.0000000000000006661338147750939242541790008544921875",1.0000000000000007));
  Assert(basic_test_64bit("1090544144181609348835077142190",0x1.b8779f2474dfbp+99));
  Assert(basic_test_64bit("2.2250738585072013e-308",2.2250738585072013e-308));
  Assert(basic_test_64bit("-92666518056446206563E3", -92666518056446206563E3));
  Assert(basic_test_64bit("-92666518056446206563E3", -92666518056446206563E3));
  Assert(basic_test_64bit("-42823146028335318693e-128",-42823146028335318693e-128));
  Assert(basic_test_64bit("90054602635948575728E72",90054602635948575728E72));
  Assert(basic_test_64bit("1.00000000000000188558920870223463870174566020691753515394643550663070558368373221972569761144603605635692374830246134201063722058e-309", 1.00000000000000188558920870223463870174566020691753515394643550663070558368373221972569761144603605635692374830246134201063722058e-309));
  Assert(basic_test_64bit("0e9999999999999999999999999999", 0));
  Assert(basic_test_32bit("1234456789012345678901234567890e9999999999999999999999999999", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-2139879401095466344511101915470454744.9813888656856943E+272", -std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("-2402844368454405395.2", -2402844368454405395.2));
  Assert(basic_test_64bit("2402844368454405395.2", 2402844368454405395.2));
  Assert(basic_test_64bit("7.0420557077594588669468784357561207962098443483187940792729600000e+59", 7.0420557077594588669468784357561207962098443483187940792729600000e+59));
  Assert(basic_test_64bit("7.0420557077594588669468784357561207962098443483187940792729600000e+59", 7.0420557077594588669468784357561207962098443483187940792729600000e+59));
  Assert(basic_test_64bit("-1.7339253062092163730578609458683877051596800000000000000000000000e+42", -1.7339253062092163730578609458683877051596800000000000000000000000e+42));
  Assert(basic_test_64bit("-2.0972622234386619214559824785284023792871122537545728000000000000e+52", -2.0972622234386619214559824785284023792871122537545728000000000000e+52));
  Assert(basic_test_64bit("-1.0001803374372191849407179462120053338028379051879898808320000000e+57", -1.0001803374372191849407179462120053338028379051879898808320000000e+57));
  Assert(basic_test_64bit("-1.8607245283054342363818436991534856973992070520151142825984000000e+58", -1.8607245283054342363818436991534856973992070520151142825984000000e+58));
  Assert(basic_test_64bit("-1.9189205311132686907264385602245237137907390376574976000000000000e+52", -1.9189205311132686907264385602245237137907390376574976000000000000e+52));
  Assert(basic_test_64bit("-2.8184483231688951563253238886553506793085187889855201280000000000e+54", -2.8184483231688951563253238886553506793085187889855201280000000000e+54));
  Assert(basic_test_64bit("-1.7664960224650106892054063261344555646357024359107788800000000000e+53", -1.7664960224650106892054063261344555646357024359107788800000000000e+53));
  Assert(basic_test_64bit("-2.1470977154320536489471030463761883783915110400000000000000000000e+45", -2.1470977154320536489471030463761883783915110400000000000000000000e+45));
  Assert(basic_test_64bit("-4.4900312744003159009338275160799498340862630046359789166919680000e+61", -4.4900312744003159009338275160799498340862630046359789166919680000e+61));
  Assert(basic_test_64bit("+1", 1));
  Assert(basic_test_64bit("1.8e308", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("1.797693134862315700000000000000001e308", 1.7976931348623157e308));
  Assert(basic_test_64bit("1.832312213213213232132132143451234453123412321321312e308", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("2e30000000000000000", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("2e3000", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("1.9e308", std::numeric_limits<double>::infinity()));
  Assert(basic_test_64bit("3e-324", 0x0.0000000000001p-1022));
  Assert(basic_test_64bit("1.00000006e+09", 0x1.dcd651ep+29));
  Assert(basic_test_64bit("4.9406564584124653e-324", 0x0.0000000000001p-1022));
  Assert(basic_test_64bit("4.9406564584124654e-324", 0x0.0000000000001p-1022));
  Assert(basic_test_64bit("2.2250738585072009e-308", 0x0.fffffffffffffp-1022));
  Assert(basic_test_64bit("2.2250738585072014e-308", 0x1p-1022));
  Assert(basic_test_64bit("1.7976931348623157e308", 0x1.fffffffffffffp+1023));
  Assert(basic_test_64bit("1.7976931348623158e308", 0x1.fffffffffffffp+1023));
  Assert(basic_test_64bit("4503599627370496.5", 4503599627370496.5));
  Assert(basic_test_64bit("4503599627475352.5", 4503599627475352.5));
  Assert(basic_test_64bit("4503599627475353.5", 4503599627475353.5));
  Assert(basic_test_64bit("2251799813685248.25", 2251799813685248.25));
  Assert(basic_test_64bit("1125899906842624.125", 1125899906842624.125));
  Assert(basic_test_64bit("1125899906842901.875", 1125899906842901.875));
  Assert(basic_test_64bit("2251799813685803.75", 2251799813685803.75));
  Assert(basic_test_64bit("4503599627370497.5", 4503599627370497.5));
  Assert(basic_test_64bit("45035996.273704995", 45035996.273704995));
  Assert(basic_test_64bit("45035996.273704985", 45035996.273704985));
  Assert(basic_test_64bit("0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000044501477170144022721148195934182639518696390927032912960468522194496444440421538910330590478162701758282983178260792422137401728773891892910553144148156412434867599762821265346585071045737627442980259622449029037796981144446145705102663115100318287949527959668236039986479250965780342141637013812613333119898765515451440315261253813266652951306000184917766328660755595837392240989947807556594098101021612198814605258742579179000071675999344145086087205681577915435923018910334964869420614052182892431445797605163650903606514140377217442262561590244668525767372446430075513332450079650686719491377688478005309963967709758965844137894433796621993967316936280457084866613206797017728916080020698679408551343728867675409720757232455434770912461317493580281734466552734375", 0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000044501477170144022721148195934182639518696390927032912960468522194496444440421538910330590478162701758282983178260792422137401728773891892910553144148156412434867599762821265346585071045737627442980259622449029037796981144446145705102663115100318287949527959668236039986479250965780342141637013812613333119898765515451440315261253813266652951306000184917766328660755595837392240989947807556594098101021612198814605258742579179000071675999344145086087205681577915435923018910334964869420614052182892431445797605163650903606514140377217442262561590244668525767372446430075513332450079650686719491377688478005309963967709758965844137894433796621993967316936280457084866613206797017728916080020698679408551343728867675409720757232455434770912461317493580281734466552734375));
  Assert(basic_test_64bit("0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000022250738585072008890245868760858598876504231122409594654935248025624400092282356951787758888037591552642309780950434312085877387158357291821993020294379224223559819827501242041788969571311791082261043971979604000454897391938079198936081525613113376149842043271751033627391549782731594143828136275113838604094249464942286316695429105080201815926642134996606517803095075913058719846423906068637102005108723282784678843631944515866135041223479014792369585208321597621066375401613736583044193603714778355306682834535634005074073040135602968046375918583163124224521599262546494300836851861719422417646455137135420132217031370496583210154654068035397417906022589503023501937519773030945763173210852507299305089761582519159720757232455434770912461317493580281734466552734375", 0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000022250738585072008890245868760858598876504231122409594654935248025624400092282356951787758888037591552642309780950434312085877387158357291821993020294379224223559819827501242041788969571311791082261043971979604000454897391938079198936081525613113376149842043271751033627391549782731594143828136275113838604094249464942286316695429105080201815926642134996606517803095075913058719846423906068637102005108723282784678843631944515866135041223479014792369585208321597621066375401613736583044193603714778355306682834535634005074073040135602968046375918583163124224521599262546494300836851861719422417646455137135420132217031370496583210154654068035397417906022589503023501937519773030945763173210852507299305089761582519159720757232455434770912461317493580281734466552734375));
  Assert(basic_test_64bit("1438456663141390273526118207642235581183227845246331231162636653790368152091394196930365828634687637948157940776599182791387527135353034738357134110310609455693900824193549772792016543182680519740580354365467985440183598701312257624545562331397018329928613196125590274187720073914818062530830316533158098624984118889298281371812288789537310599037529113415438738954894752124724983067241108764488346454376699018673078404751121414804937224240805993123816932326223683090770561597570457793932985826162604255884529134126396282202126526253389383421806727954588525596114379801269094096329805054803089299736996870951258573010877404407451953846698609198213926882692078557033228265259305481198526059813164469187586693257335779522020407645498684263339921905227556616698129967412891282231685504660671277927198290009824680186319750978665734576683784255802269708917361719466043175201158849097881370477111850171579869056016061666173029059588433776015644439705050377554277696143928278093453792803846252715966016733222646442382892123940052441346822429721593884378212558701004356924243030059517489346646577724622498919752597382095222500311124181823512251071356181769376577651390028297796156208815375089159128394945710515861334486267101797497111125909272505194792870889617179758703442608016143343262159998149700606597792535574457560429226974273443630323818747730771316763398572110874959981923732463076884528677392654150010269822239401993427482376513231389212353583573566376915572650916866553612366187378959554983566712767093372906030188976220169058025354973622211666504549316958271880975697143546564469806791358707318873075708383345004090151974068325838177531266954177406661392229801349994695941509935655355652985723782153570084089560139142231.738475042362596875449154552392299548947138162081694168675340677843807613129780449323363759027012972466987370921816813162658754726545121090545507240267000456594786540949605260722461937870630634874991729398208026467698131898691830012167897399682179601734569071423681e-733", std::numeric_limits<double>::infinity()));

  std::cout << std::endl;

  std::cout << "======= 32 bits " << std::endl;
  Assert(basic_test_32bit("INF",std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("-INF",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("INFINITY",std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("-INFINITY",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("infinity",std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("-infinity",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("inf",std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("-inf",-std::numeric_limits<double>::infinity()));
  Assert(basic_test_32bit("-0",-0.0f));
  Assert(basic_test_32bit("1090544144181609348835077142190",0x1.b877ap+99f));
  Assert(basic_test_32bit("1.1754943508e-38",1.1754943508e-38f));
  Assert(basic_test_32bit("30219.0830078125",30219.0830078125f));
  Assert(basic_test_32bit("16252921.5",16252921.5f));
  Assert(basic_test_32bit("5322519.25",5322519.25f));
  Assert(basic_test_32bit("3900245.875",3900245.875f));
  Assert(basic_test_32bit("1510988.3125",1510988.3125f));
  Assert(basic_test_32bit("782262.28125",782262.28125f));
  Assert(basic_test_32bit("328381.484375",328381.484375f));
  Assert(basic_test_32bit("156782.0703125",156782.0703125f));
  Assert(basic_test_32bit("85003.24609375",85003.24609375f));
  Assert(basic_test_32bit("43827.048828125",43827.048828125f));
  Assert(basic_test_32bit("17419.6494140625",17419.6494140625f));
  Assert(basic_test_32bit("15498.36376953125",15498.36376953125f));
  Assert(basic_test_32bit("6318.580322265625",6318.580322265625f));
  Assert(basic_test_32bit("2525.2840576171875",2525.2840576171875f));
  Assert(basic_test_32bit("1370.9265747070312",1370.9265747070312f));
  Assert(basic_test_32bit("936.3702087402344",936.3702087402344f));
  Assert(basic_test_32bit("411.88682556152344",411.88682556152344f));
  Assert(basic_test_32bit("206.50310516357422",206.50310516357422f));
  Assert(basic_test_32bit("124.16878890991211",124.16878890991211f));
  Assert(basic_test_32bit("50.811574935913086",50.811574935913086f));
  Assert(basic_test_32bit("17.486443519592285",17.486443519592285f));
  Assert(basic_test_32bit("13.91745138168335",13.91745138168335f));
  Assert(basic_test_32bit("7.5464513301849365",0x1.e2f90ep+2f));
  Assert(basic_test_32bit("2.687217116355896",2.687217116355896f));
  Assert(basic_test_32bit("1.1877630352973938",0x1.30113ep+0f));
  Assert(basic_test_32bit("0.7622503340244293",0.7622503340244293f));
  Assert(basic_test_32bit("0.30531780421733856",0x1.38a53ap-2f));
  Assert(basic_test_32bit("0.21791061013936996",0x1.be47eap-3f));
  Assert(basic_test_32bit("0.09289376810193062",0x1.7c7e2ep-4f));
  Assert(basic_test_32bit("0.03706067614257336",0.03706067614257336f));
  Assert(basic_test_32bit("0.028068351559340954",0.028068351559340954f));
  Assert(basic_test_32bit("0.012114629615098238",0x1.8cf8e2p-7f));
  Assert(basic_test_32bit("0.004221370676532388",0x1.14a6dap-8f));
  Assert(basic_test_32bit("0.002153817447833717",0.002153817447833717f));
  Assert(basic_test_32bit("0.0015924838953651488",0x1.a175cap-10f));
  Assert(basic_test_32bit("0.0008602388261351734",0.0008602388261351734f));
  Assert(basic_test_32bit("0.00036393293703440577",0x1.7d9c82p-12f));
  Assert(basic_test_32bit("0.00013746770127909258",0.00013746770127909258));
  Assert(basic_test_32bit("16407.9462890625", 16407.9462890625f));
  Assert(basic_test_32bit("1.1754947011469036e-38", 0x1.000006p-126f));
  Assert(basic_test_32bit("7.0064923216240854e-46", 0x1p-149f));
  Assert(basic_test_32bit("8388614.5", 8388614.5f));
  Assert(basic_test_32bit("0e9999999999999999999999999999", 0));
  Assert(basic_test_32bit("1234456789012345678901234567890e9999999999999999999999999999", std::numeric_limits<float>::infinity()));
  Assert(basic_test_32bit("4.7019774032891500318749461488889827112746622270883500860350068251e-38",4.7019774032891500318749461488889827112746622270883500860350068251e-38f));
  Assert(basic_test_32bit("3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679", 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679));
  Assert(basic_test_32bit("2.3509887016445750159374730744444913556373311135441750430175034126e-38", 2.3509887016445750159374730744444913556373311135441750430175034126e-38f));
  Assert(basic_test_32bit("+1", 1));
  Assert(basic_test_32bit("2e3000", std::numeric_limits<float>::infinity()));
  Assert(basic_test_32bit("3.5028234666e38", std::numeric_limits<float>::infinity()));
  Assert(basic_test_32bit("7.0060e-46", 0));
  Assert(basic_test_32bit(1.00000006e+09f));
  Assert(basic_test_32bit(1.4012984643e-45f));
  Assert(basic_test_32bit(1.1754942107e-38f));
  Assert(basic_test_32bit(1.1754943508e-45f));
  Assert(basic_test_32bit("3.4028234664e38", 0x1.fffffep+127f));
  Assert(basic_test_32bit("3.4028234665e38", 0x1.fffffep+127f));
  Assert(basic_test_32bit("3.4028234666e38", 0x1.fffffep+127f));
  Assert(basic_test_32bit("0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625", 0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625));
  Assert(basic_test_32bit("0.00000000000000000000000000000000000000000000140129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125", 0.00000000000000000000000000000000000000000000140129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125));
  Assert(basic_test_32bit("0.00000000000000000000000000000000000002350988561514728583455765982071533026645717985517980855365926236850006129930346077117064851336181163787841796875", 0.00000000000000000000000000000000000002350988561514728583455765982071533026645717985517980855365926236850006129930346077117064851336181163787841796875));
  Assert(basic_test_32bit("0.00000000000000000000000000000000000001175494210692441075487029444849287348827052428745893333857174530571588870475618904265502351336181163787841796875", 0.00000000000000000000000000000000000001175494210692441075487029444849287348827052428745893333857174530571588870475618904265502351336181163787841796875));
  std::cout << std::endl;

  Assert(issue8());

  std::cout << "All ok" << std::endl;
  return EXIT_SUCCESS;
}
