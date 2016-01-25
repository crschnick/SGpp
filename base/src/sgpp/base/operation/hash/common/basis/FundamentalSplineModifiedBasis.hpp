// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef FUNDAMENTAL_SPLINE_MODIFIED_BASE_HPP
#define FUNDAMENTAL_SPLINE_MODIFIED_BASE_HPP

#include <cmath>
#include <stdexcept>
#include <sgpp/base/operation/hash/common/basis/Basis.hpp>
#include <sgpp/base/operation/hash/common/basis/FundamentalSplineBasis.hpp>
#include <sgpp/base/operation/hash/common/basis/BsplineBasis.hpp>
#include <sgpp/base/operation/hash/common/basis/BsplineModifiedBasis.hpp>

#include <sgpp/globaldef.hpp>

namespace SGPP {
  namespace base {

    /**
     * Modified fundamental spline basis on Noboundary grids.
     */
    template <class LT, class IT>
    class FundamentalSplineModifiedBasis: public Basis<LT, IT> {
      public:
        /**
         * Default constructor.
         */
        FundamentalSplineModifiedBasis() :
          fundamentalSplineBasis(FundamentalSplineBasis<LT, IT>()),
          bsplineBasis(BsplineBasis<LT, IT>()) {
        }

        /**
         * Constructor.
         *
         * @param degree    fundamental spline degree, must be odd
         *                  (if it's even, degree - 1 is used)
         */
        FundamentalSplineModifiedBasis(size_t degree) :
          fundamentalSplineBasis(FundamentalSplineBasis<LT, IT>(degree)),
          bsplineBasis(BsplineBasis<LT, IT>(degree)) {
          switch (bsplineBasis.getDegree()) {
            case 1:
              coefficients = {2.0, 1.0};
              break;
            case 3:
              coefficients = {
                3.53589838486224541294510731698825526611438949237924,
                2.26794919243112270647255365849412763305719474618962,
                1.00000000000000000000000000000000000000000000000000,
                -0.267949192431122706472553658494127633057194746189619,
                0.0717967697244908258902146339765105322287789847584775,
                -0.0192378864668405970883048774119144958579211928442906,
                0.00515477614287156246300487567114745120290578661868483,
                -0.00138121810464565276371462527267530895370195363044874,
                0.000370096275711048591853625419553784611902027903110116,
                -0.0000991669981985416036998764055398294939061579819917295,
                0.0000265717170831178229458802026055333637226040248568017,
                -7.11987013392968808364440488230396098425811743547731e-6,
                1.90776345260092938869741692368248021442844488510751e-6,
                -5.11183676474029471145262812425959873455662104952738e-7,
                1.36971253295188495883634326021359279394203534703442e-7,
                -3.67013367067245123892744916594772441211520338610312e-8,
                9.83409353170955367346364061654969709040460074068231e-9,
                -2.63503742011370230458007080672154424046636910169807e-9,
                7.06056148745255544856642610336479871460875666109964e-10,
                -1.89187174867319874846499634624375245377133562741787e-10
              };
              break;
            case 5:
              coefficients = {
                4.13498460921696645101507051173915074610343417519976,
                3.50566336646333315977845308437468054404087224639879,
                2.14280042902966070542111905133624555379257597124673,
                1.14670833893600783262414332113479295763714686927022,
                -0.582925445837684203293549014740624426704016579057380,
                0.254836700768760831711266769242834947604890184843806,
                -0.109892036066488560767546428273358927620445160424252,
                0.0473239398342691400207314272016677128761061575610139,
                -0.0203768294528500510803398267747555228525077354672960,
                0.00877377367227983496550166669807171486887870930952988,
                -0.00377777121568141757208544598600168548365083736834864,
                0.00162661517707993721921566250528652251802192173677489,
                -0.000700380395530465964671297436447407285953405425617921,
                0.000301566531953280521939936936694210951384768706457901,
                -0.000129847114171490029654508699968503847998356064921153,
                0.0000559089662544041852194592197092441733291423910195572,
                -0.0000240730225509944642754284321211719887167828239998759,
                0.0000103652500406400957541199968272453634854445981980854,
                -4.46302113402663348027440998287285387323927074574191e-6,
                1.92166687389803665689488439772774397208859129287340e-6,
                -8.27422381239168712847420858520449545288233546085311e-7,
                3.56267679000341911985234798949423238423488059778346e-7,
                -1.53400079546074262678912468383823611051378381572866e-7,
                6.60502924957145157642853711214243567911068808683530e-8,
                -2.84396276173970718318806499892296009002617269197299e-8,
                1.22454025327547448525517847359527241061006237178418e-8,
                -5.27256844591977245473068461882683285874276096602560e-9,
                2.27023798871027541919153611516312994036863878373344e-9,
                -9.77508509988473221220507297211635630916874363535801e-10,
                4.20891065981465055537821480280880560719735124347568e-10,
                -1.81225316826247287679131246749488529041252650717738e-10
              };
              break;
            case 7:
              coefficients = {
                24.9115245321977719353935828370258928395024876082414,
                2.09378156609051971900149703025258131515629353290686,
                4.25008115228175382614251351171872841329462314192185,
                1.68777131570104398194127761877502337437983898241899,
                1.59346722083377338106108855426010308524801193411935,
                -1.18624467640187099843998862113131931617957267927211,
                0.676069774625269218378393856383216657303804357425610,
                -0.366925734644797055135524488546577062681593334900993,
                0.197025716169211255756068437941368693550289047272929,
                -0.105539694127980193763560078428493652518561409607023,
                0.0565026083518101966473271808664837300323308562155029,
                -0.0302458772843492724981626478702689782706904600100818,
                0.0161901655358884179810299967267362549143115877483808,
                -0.00866629585620457554698962322441524118285477033965892,
                0.00463890067175339469745450145254798115665316970102827,
                -0.00248311300643558800235901788119819396394835392347686,
                0.00132916183122868486716508796927447538242583418131006,
                -0.000711474321469871061091468221339871969337171842410807,
                0.000380838281769027226157176348168202204850929355135214,
                -0.000203855279586949787735298505822759595391311272840020,
                0.000109119741884520564033885672868117436088453215191482,
                -0.0000584096624452111845767042995952775859880653996494599,
                0.0000312655492764536639419375564739613632060287883160784,
                -0.0000167358366858004330746447322091987643631563393937064,
                8.95836587091568914813389144159875312312348850274155e-6,
                -4.79523794261605507148877330250334745835454170122222e-6,
                2.56679703169497181586162485771073084729403087040722e-6,
                -1.37395622089270620567411867074606645756174129242384e-6,
                7.35451877814894294540935489910385897448070752255753e-7,
                -3.93672997986806067015815660971671311516308677361736e-7,
                2.10725451955302891638118681150017742702486831716732e-7,
                -1.12797210702408665668805908453128603809005988935611e-7,
                6.03781395374219162464438208098297212833248874512279e-8,
                -3.23192365422786591218430891859750069641533398987132e-8,
                1.72998548593629072992495175875495156640830583934433e-8,
                -9.26027376183563121368307591260588215224557634598698e-9,
                4.95684332852832971269688477506619367930669636264869e-9,
                -2.65330123228509480325062084417209850034389985073052e-9,
                1.42026022665028579494757378832438630445841988097279e-9,
                -7.60237505964411889448655156242201231736314899927344e-10,
                4.06940259700240125508021195829706580352484328015520e-10,
                -2.17827157520758959406196693188479670862770930470581e-10,
                1.16598614716875451458451061836736930643682700866907e-10
              };
              break;
            case 9:
              coefficients = {
                666.971066320491959931552562581856322052395644973322,
                58.4336977902353730685493872076482604250843671473768,
                -6.29413052849360633414044570767134660804999408822057,
                7.47188730926041184373496953389458180219938714463637,
                -0.184194843155011138668838354264985340565403661524490,
                3.08201697804124151855635462231713584187193138817045,
                -2.64807650466798452000042399286776834991609704311141,
                1.77380539465399346727316209006127691834116992483548,
                -1.11183990085070568491818764045024606032099730693053,
                0.682742511662640998763268000149053408465296874783484,
                -0.416467438229122456053219562266136244660124663999708,
                0.253485880130727095282843089477482207237688584168560,
                -0.154174188439075703628377264908851648239353073935585,
                0.0937486881636065612923952759772262857960384644366062,
                -0.0570012140447050438916880326849759497118657395748903,
                0.0346570445492442209075916410621919865082117890567353,
                -0.0210714844452599379801366189582426636603251523367062,
                0.0128114260580023223343042617392354728808897807434434,
                -0.00778931733310376147574820765068394127368766557334601,
                0.00473588535613928999919275732077254003278121547020955,
                -0.00287940608409612514478932059568603266635011267489548,
                0.00175067141218536427189445341083563875463039247093949,
                -0.00106440365409432395563316940671987973283211237896271,
                0.000647154643960430089512564655458930373326445813818149,
                -0.000393468334168387621722029137935477967020639502744892,
                0.000239227719945768271715811223250837257408696462017333,
                -0.000145449829154051364658718563021091217737060553923363,
                0.0000884331163827560760919786054583448621064830953160787,
                -0.0000537671038771785812863175431235583519428503289500454,
                0.0000326902587805671368577265846678538447817798480116135,
                -0.0000198755919898485636001042882610852897028529260938176,
                0.0000120843080380122081355202471023224290509985943683499,
                -7.34722773702155311284257575397757868982557989080402e-6,
                4.46709528173655453325335616096982118216440885914424e-6,
                -2.71598226846334012888051765195178380886855522694493e-6,
                1.65131012825399814867686970856716808353504804063511e-6,
                -1.00399224668614060637509576461175626493198561658088e-6,
                6.10424664730716597166619651828557298095789877017265e-7,
                -3.71136602440409460454593147302600990250231804929448e-7,
                2.25650085308683188166283943026835083843059900727368e-7,
                -1.37194662733357060750951390513024407202148316153687e-7,
                8.34139967497512534035954201992983922790405687795762e-8,
                -5.07154922439689987282462034610893231411149021744968e-8,
                3.08348868747348416267143436049654994199692160629908e-8,
                -1.87475307151487105993846524075613210537081665561363e-8,
                1.13984497281690357630904688649957142931060221807436e-8,
                -6.93022767529660595962755635898007224634723424019808e-9,
                4.21356033292449126448899047069027441109514880133086e-9,
                -2.56183368152257631650325180255383200112995833265541e-9,
                1.55758818984997514522322830270236179826692122213601e-9,
                -9.47009552828670711897508646064352849702095310354260e-10,
                5.75779335637579580747987250174360267343204937449277e-10,
                -3.50072332804894274382934361697345567892245436217477e-10,
                2.12843064365545985074150819517502832636811170983817e-10,
                -1.29408027436901728019547677545317904349056783015620e-10
              };
              break;
            case 11:
              coefficients = {
                25478.1030065839813849765713666014144774849779734540,
                -1257.20577541994216521227015666434867652083998134581,
                184.338940247187516142051522617641794228555451476932,
                -40.8752405741447510612822684267244758404012290153643,
                21.1329442625005613607963595027781013482712553783829,
                -7.64827760878038446164980439121884335580289620278008,
                8.29412744707749665988143692233346401925561022366278,
                -6.90314801653199977351869115238663980735187906814267,
                5.00070913128874638244693089904882879935619290843830,
                -3.42993592905690614738916376137499869558739044079142,
                2.30201957418836189963421142523765024987843857126920,
                -1.53151587353998509592140647823263737573844102760067,
                1.01526542344197890246944794576661104600055993347935,
                -0.672048379498278681584144867666411344908360216040798,
                0.444590022995201975650964005133381295916120308568944,
                -0.294043260268735151861459573756723708266391892491243,
                0.194454702276536272805869844884776612301180160153989,
                -0.128590072114694701711442772553840566529670421968772,
                0.0850332791221676711165977417078589844085881716578511,
                -0.0562299019131210861014004381268789847223193676868745,
                0.0371830023222377532324341759754976377909331344862359,
                -0.0245878784965011785676267336715433429031274464707610,
                0.0162591353958252518451280637054526922137287856712185,
                -0.0107516160819744284200371774162341505229466268211157,
                0.00710967931866821444915801285042230557792476148032517,
                -0.00470138980792054570029750526686034361129858591103517,
                0.00310886958760593836439802908180688164681167645144786,
                -0.00205578997934584554153810954719680230077700535698419,
                0.00135942416042056545921092309354754711039371725766166,
                -0.000898941071154102852115927814448261686456444493442251,
                0.000594439228465360744489637172242679168591063045817115,
                -0.000393082491853910504088630293355551476998659508821086,
                0.000259932114154524917760530846516009154978242942738209,
                -0.000171884287311444952126998005168603701115193586849357,
                0.000113661246977175920708995530688382812380438786831625,
                -0.0000751603259752063858355629217011896633760750549925267,
                0.0000497009732949915719356952205639707728209711108122426,
                -0.0000328655672313382707522139089952357908833916392375843,
                0.0000217328844452649782657969933393615469249925869572836,
                -0.0000143712190629956893132490554025217899622455546661588,
                9.50319953509883860653497151217281710009175941610385e-6,
                -6.28414339855420181265774933466743087914916796680949e-6,
                4.15549080157046806011272216204805885391096160025384e-6,
                -2.74788506670767407115654013489546978157123182951468e-6,
                1.81708315585281886889169577382952464456825683915316e-6,
                -1.20157543533653515016948798721620906654729201743717e-6,
                7.94561064612679707183921974995007851956427531450731e-7,
                -5.25416271698009422514527132290955695739566210080715e-7,
                3.47439952522223057163598664877292177979105955237200e-7,
                -2.29750251583428350540900603097583472594029560335754e-7,
                1.51926045693528465690212292427473203763253824123895e-7,
                -1.00463538999392964023019046187141591476911064498933e-7,
                6.64331295019842361247970032457141724711684016054284e-8,
                -4.39299743905505394683746340531843748597665327462322e-8,
                2.90494014721493042014147798962026722849435633209985e-8,
                -1.92093835154073860203999673343510052000016069558763e-8,
                1.27025135232400180028867047209917190166761624187349e-8,
                -8.39974118267134812048927546863792647579746622682507e-9,
                5.55446383164869052021504427518899863574716357936221e-9,
                -3.67297846280564188910141850301746214034551728187193e-9,
                2.42881602925655023940653860031534753926317980171058e-9,
                -1.60609362774957083430836167770804432505774196682452e-9,
                1.06205521950847866735194878463232182239086859878568e-9,
                -7.02301079959878577888085853554332310316634300018734e-10,
                4.64407874329809462377826525328669933978828868680176e-10,
                -3.07097169424619528947000094175953873782028658169157e-10,
                2.03072937995961057197411295185802301051814943418342e-10,
                -1.34285243408711811461226717355585409911562637356729e-10
              };
              break;
            default:
              throw std::invalid_argument("Degree is unsupported.");
          }
        }

        /**
         * @param l     level of basis function
         * @param i     index of basis function
         * @param x     evaluation point
         * @return      value of modified fundamental spline basis function
         */
        inline float_t eval(LT l, IT i, float_t x) {
          if (l == 1) {
            return 1.0;
          }

          const IT hInv = static_cast<IT>(1) << l;

          if ((i != 1) && (i != hInv - 1)) {
            return fundamentalSplineBasis.eval(l, i, x);
          }

          if (i == hInv - 1) {
            // mirror the situation at x = 0.5
            x = 1.0 - x;
            i = 1;
          }

          const size_t p = bsplineBasis.getDegree();
          const float_t t = x * static_cast<float_t>(hInv) +
                            static_cast<float_t>(p);

          /*float_t y = 0.0;

          for (size_t k = 0; k < coefficients.size(); k++) {
            y += coefficients[k] * bsplineBasis.uniformBSpline(
                   t - static_cast<float_t>(k), p);
          }*/

          const int kMin = std::max(static_cast<int>(std::floor(t)) -
                                    static_cast<int>(p + 1) + 1,
                                    0);
          const int kMax = std::min(static_cast<int>(std::floor(t)),
                                    static_cast<int>(coefficients.size()) - 1);

          float_t y = 0;

          for (int k = kMin; k <= kMax; k++) {
            y += coefficients[k] * bsplineBasis.uniformBSpline(
                   t - static_cast<float_t>(k), p);
          }

          return y;
        }

        /**
         * @param l     level of basis function
         * @param i     index of basis function
         * @param x     evaluation point
         * @return      value of derivative of modified
         *              fundamental spline basis function
         */
        inline float_t evalDx(LT l, IT i, float_t x) {
          if (l == 1) {
            return 0.0;
          }

          const IT hInv = static_cast<IT>(1) << l;

          if ((i != 1) && (i != hInv - 1)) {
            return fundamentalSplineBasis.evalDx(l, i, x);
          }

          const float_t hInvDbl = static_cast<float_t>(hInv);
          // inner derivative
          float_t dxFactor = hInvDbl;

          if (i == hInv - 1) {
            // mirror the situation at x = 0.5
            x = 1.0 - x;
            i = 1;
            dxFactor *= -1.0;
          }

          const size_t p = bsplineBasis.getDegree();
          const float_t t = x * hInvDbl + static_cast<float_t>(p);

          /*float_t y = 0.0;

          for (size_t k = 0; k < coefficients.size(); k++) {
            y += coefficients[k] * bsplineBasis.uniformBSplineDx(
                   t - static_cast<float_t>(k), p);
          }*/

          const int kMin = std::max(static_cast<int>(std::floor(t)) -
                                    static_cast<int>(p + 1) + 1,
                                    0);
          const int kMax = std::min(static_cast<int>(std::floor(t)),
                                    static_cast<int>(coefficients.size()) - 1);

          float_t y = 0;

          for (int k = kMin; k <= kMax; k++) {
            y += coefficients[k] * bsplineBasis.uniformBSplineDx(
                   t - static_cast<float_t>(k), p);
          }

          return dxFactor * y;
        }

        /**
         * @param l     level of basis function
         * @param i     index of basis function
         * @param x     evaluation point
         * @return      value of 2nd derivative of modified
         *              fundamental spline basis function
         */
        inline float_t evalDxDx(LT l, IT i, float_t x) {
          if (l == 1) {
            return 0.0;
          }

          const IT hInv = static_cast<IT>(1) << l;

          if ((i != 1) && (i != hInv - 1)) {
            return fundamentalSplineBasis.evalDxDx(l, i, x);
          }

          const float_t hInvDbl = static_cast<float_t>(hInv);
          // inner derivative
          const float_t dxFactor = hInvDbl * hInvDbl;

          if (i == hInv - 1) {
            // mirror the situation at x = 0.5
            x = 1.0 - x;
            i = 1;
          }

          const size_t p = bsplineBasis.getDegree();
          const float_t t = x * hInvDbl + static_cast<float_t>(p);

          /*float_t y = 0.0;

          for (size_t k = 0; k < coefficients.size(); k++) {
            y += coefficients[k] * bsplineBasis.uniformBSplineDxDx(
                   t - static_cast<float_t>(k), p);
          }*/

          const int kMin = std::max(static_cast<int>(std::floor(t)) -
                                    static_cast<int>(p + 1) + 1,
                                    0);
          const int kMax = std::min(static_cast<int>(std::floor(t)),
                                    static_cast<int>(coefficients.size()) - 1);

          float_t y = 0;

          for (int k = kMin; k <= kMax; k++) {
            y += coefficients[k] * bsplineBasis.uniformBSplineDxDx(
                   t - static_cast<float_t>(k), p);
          }

          return dxFactor * y;
        }

        /**
         * @return      B-spline degree
         */
        inline size_t getDegree() const {
          return bsplineBasis.getDegree();
        }

      protected:
        /// fundamental spline basis for fundamental spline evaluation
        FundamentalSplineBasis<LT, IT> fundamentalSplineBasis;
        /// B-spline basis
        BsplineBasis<LT, IT> bsplineBasis;
        /// B-spline coefficients
        std::vector<float_t> coefficients;
    };

    // default type-def (unsigned int for level and index)
    typedef FundamentalSplineModifiedBasis<unsigned int, unsigned int>
    SFundamentalSplineModifiedBase;
  }
}

#endif /* FUNDAMENTAL_SPLINE_MODIFIED_BASE_HPP */
