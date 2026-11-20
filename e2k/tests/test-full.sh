#!/bin/bash
# Runtime smoke tests of a full ROOT build on e2k: JIT, ACLiC, C++ exceptions,
# pointer checks, clad, MathMore/GSL, fitting, RDataFrame (IMT), RNTuple,
# graphics, RooFit, SQLite, XML/GDML, THttpServer, TMVA and PyROOT.
#
# Usage: bash test-full.sh ROOT_DIR   (build or install directory containing
#                                       bin/thisroot.sh)
# Exit status is 0 only if all checks pass; logs are kept in a temp directory.
B=${1:?usage: $0 ROOT_DIR}
. "$B/bin/thisroot.sh" || exit 1
W=$(mktemp -d "${TMPDIR:-/tmp}/root-e2k-test.XXXXXX"); cd "$W" || exit 1
echo "work dir: $W"
pass=0; fail=0; failed=""
# run NAME EXPECTED-REGEX TIMEOUT CMD...  (output in $W/NAME.log)
run() {
  local n=$1 re=$2 t=$3; shift 3
  timeout "$t" "$@" > "$n.log" 2>&1
  local rc=$?
  if [ $rc = 0 ] && grep -qE "$re" "$n.log"; then echo "PASS $n"; pass=$((pass+1))
  else echo "FAIL $n (rc=$rc, log $W/$n.log)"; fail=$((fail+1)); failed="$failed $n"; tail -5 "$n.log" | sed 's/^/      /'; fi
}

# --- core / minimal build regressions ---
run version   '6\.[0-9]+'               120 root -b -l -q -e 'printf("v=%s\n", gROOT->GetVersion()); gSystem->Exit(0);'
cat > eh.C <<'EOF'
#include <stdexcept>
void eh() { try { throw std::runtime_error("x"); } catch (const std::exception &e) { printf("caught %s\n", e.what()); } }
EOF
run eh-jit    'caught x'                120 root -b -l -q eh.C
run eh-aclic  'caught x'                600 root -b -l -q eh.C+

# --- null pointer dereference in interpreted code must throw, not crash ---
cat > nullderef.C <<'EOF'
int deref(int *p) { return *p; }
EOF
printf '.L nullderef.C\nint *p = nullptr;\nderef(p)\nprintf("survived\\n");\n.q\n' > nullderef.in
# ROOT enables cling's pointer checks only for a terminal or with --ptrcheck.
# On e2k the default is an inline null check; CLING_POINTER_CHECKS=1 = full one.
for v in inline full; do
  [ $v = full ] && export CLING_POINTER_CHECKS=1
  run nullderef-$v 'survived' 120 bash -c 'root -b -l --ptrcheck < nullderef.in'
  unset CLING_POINTER_CHECKS
  grep -q 'dereference null pointer' nullderef-$v.log || { echo "FAIL nullderef-$v-message (no cling null-deref exception, log $W/nullderef-$v.log)"; fail=$((fail+1)); failed="$failed nullderef-$v-message"; }
done

# --- clad (automatic differentiation in cling) ---
cat > cladtest.C <<'EOF'
#include "Math/CladDerivator.h"
double f(double x, double y) { return x * x * y + y; }
void cladtest() {
  auto d = clad::differentiate(f, "x");
  auto g = clad::gradient(f);
  double dx = 0, dy = 0;
  g.execute(3., 2., &dx, &dy);
  printf("clad d/dx=%g grad=(%g,%g)\n", d.execute(3., 2.), dx, dy);
}
EOF
run clad      'clad d/dx=12 grad=\(12,10\)' 300 root -b -l -q cladtest.C

# --- MathMore (GSL) ---
run mathmore  'bessel=0\.76519[78]'        120 root -b -l -q -e 'gSystem->Load("libMathMore"); printf("bessel=%.6f\n", ROOT::Math::cyl_bessel_j(0, 1.0)); gSystem->Exit(0);'
cat > integ.C <<'EOF'
#include "Math/Integrator.h"
#include "Math/Functor.h"
void integ() {
  ROOT::Math::Functor1D fn([](double x) { return x * x; });
  ROOT::Math::Integrator ig(fn, ROOT::Math::IntegrationOneDim::kADAPTIVE);
  printf("integral=%.6f\n", ig.Integral(0, 3));
}
EOF
run gsl-integ 'integral=9\.000000'      120 root -b -l -q integ.C

# --- fit (Minuit2) ---
run fit       'mean=-?0\.[0-9]'         120 root -b -l -q -e 'TH1F h("h","h",100,-5,5); h.FillRandom("gaus",10000); h.Fit("gaus","Q"); printf("mean=%.3f sigma=%.3f\n", h.GetFunction("gaus")->GetParameter(1), h.GetFunction("gaus")->GetParameter(2)); gSystem->Exit(0);'

# --- RDataFrame + implicit MT ---
run rdf-mt    'n=1000000 mean='         300 root -b -l -q -e 'ROOT::EnableImplicitMT(4); ROOT::RDataFrame d(1000000); auto x = d.Define("x","(double)rdfentry_ * 1e-6"); printf("n=%llu mean=%.4f pool=%u\n", *x.Count(), *x.Mean("x"), ROOT::GetThreadPoolSize()); gSystem->Exit(0);'

# --- RNTuple ---
cat > ntup.C <<'EOF'
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleView.hxx>
void ntup() {
  { auto m = ROOT::RNTupleModel::Create(); auto px = m->MakeField<float>("px");
    auto w = ROOT::RNTupleWriter::Recreate(std::move(m), "nt", "nt.root");
    for (int i = 0; i < 1000; ++i) { *px = i; w->Fill(); } }
  auto r = ROOT::RNTupleReader::Open("nt", "nt.root");
  auto v = r->GetView<float>("px");
  double s = 0; for (auto i : r->GetEntryRange()) s += v(i);
  printf("ntuple n=%llu sum=%.0f\n", (unsigned long long)r->GetNEntries(), s);
}
EOF
run rntuple   'ntuple n=1000 sum=499500' 300 root -b -l -q ntup.C

# --- graphics in batch: PNG via libAfterImage, PDF, SVG ---
run graphics  'ok'                      120 root -b -l -q -e 'TCanvas c("c","c",600,400); TH1F h("h","h",50,-3,3); h.FillRandom("gaus",5000); h.Draw(); c.SaveAs("h.png"); c.SaveAs("h.pdf"); c.SaveAs("h.svg"); printf("ok\n"); gSystem->Exit(0);'
for f in h.png h.pdf h.svg; do [ -s "$f" ] || { echo "FAIL graphics-file $f empty"; fail=$((fail+1)); failed="$failed $f"; }; done
file h.png 2>/dev/null | grep -q PNG || { echo "FAIL h.png is not a PNG"; fail=$((fail+1)); failed="$failed png"; }

# --- RooFit ---
cat > rf.C <<'EOF'
using namespace RooFit;
void rf() {
  RooRealVar x("x","x",-10,10), m("m","m",1,-5,5), s("s","s",2,0.1,10);
  RooGaussian g("g","g",x,m,s);
  std::unique_ptr<RooDataSet> d{g.generate(x, 10000)};
  m.setVal(0); s.setVal(1);
  std::unique_ptr<RooFitResult> r{g.fitTo(*d, Save(), PrintLevel(-1))};
  printf("roofit status=%d m=%.2f s=%.2f\n", r->status(), m.getVal(), s.getVal());
}
EOF
run roofit    'roofit status=0 m=1\.0[0-9]|roofit status=0 m=0\.9[0-9]' 300 root -b -l -q rf.C

# --- SQLite ---
run sqlite    'sqlite rows=1'           120 root -b -l -q -e 'auto s = TSQLServer::Connect("sqlite://test.db","",""); s->Exec("create table t(a int)"); s->Exec("insert into t values (42)"); auto r = s->Query("select a from t"); printf("sqlite rows=%d\n", r->GetRowCount() < 0 ? (r->Next() ? 1 : 0) : r->GetRowCount()); gSystem->Exit(0);'

# --- XML / GDML / Geom ---
run xml       'xml ok'                  120 root -b -l -q -e 'TXMLEngine x; auto n = x.NewChild(nullptr, nullptr, "root"); x.NewAttr(n, nullptr, "a", "1"); auto d = x.NewDoc(); x.DocSetRootElement(d, n); x.SaveDoc(d, "t.xml"); printf("xml %s\n", gSystem->AccessPathName("t.xml") ? "fail" : "ok"); gSystem->Exit(0);'
run geom      'geom ok'                 120 root -b -l -q -e 'new TGeoManager("g","g"); auto m = new TGeoMaterial("v",0,0,0); auto md = new TGeoMedium("v",1,m); auto top = gGeoManager->MakeBox("top",md,10,10,10); gGeoManager->SetTopVolume(top); gGeoManager->CloseGeometry(); gGeoManager->Export("g.gdml"); printf("geom %s\n", gSystem->AccessPathName("g.gdml") ? "fail" : "ok"); gSystem->Exit(0);'

# --- THttpServer (civetweb) ---
cat > http.C <<'EOF'
void http() {
  auto s = new THttpServer("http:18089");
  TH1F *h = new TH1F("hh","hh",10,0,1); s->Register("/", h);
  gSystem->Exec("(sleep 2; curl -s -o http.out -w 'code=%{http_code}\\n' http://localhost:18089/hh/root.json > http.code) &");
  for (int i = 0; i < 60; ++i) { gSystem->ProcessEvents(); gSystem->Sleep(100); }
  gSystem->Exec("cat http.code; grep -c TH1F http.out");
}
EOF
run http      'code=200'                120 root -b -l -q http.C

# --- TMVA (BDT on generated data) ---
run tmva      'BDT|ROC'                 900 root -b -l -q "$ROOTSYS/tutorials/machine_learning/TMVAMinimalClassification.C"

# --- PyROOT / cppyy ---
run pyroot    'py entries=1000'         300 python3 -c 'import ROOT; h = ROOT.TH1F("h","h",10,0,1); h.FillRandom("gaus",1000); print("py entries=%d" % h.GetEntries())'
run py-jit    'py jit=42'               300 python3 -c 'import ROOT; ROOT.gInterpreter.Declare("int answer(){ return 42; }"); print("py jit=%d" % ROOT.answer())'
run py-eh     'py caught: .*boom'       300 python3 -c '
import ROOT
ROOT.gInterpreter.Declare("#include <stdexcept>\nvoid boom(){ throw std::runtime_error(\"boom\"); }")
try:
    ROOT.boom()
except Exception as e:
    print("py caught:", type(e).__name__, e)'
run py-rdf    'py rdf=499500'           300 python3 -c 'import ROOT; d = ROOT.RDataFrame(1000).Define("x","(int)rdfentry_"); print("py rdf=%d" % d.Sum("x").GetValue())'
run py-numpy  'py numpy=4950'           300 python3 -c 'import ROOT, numpy as np; v = ROOT.std.vector("double")(range(100)); print("py numpy=%d" % np.asarray(v).sum())'

echo
echo "passed: $pass  failed: $fail${failed:+ ($failed )}"
echo "logs: $W"
[ "$fail" = 0 ]
