"""
Top pT Reweighting Scale Factor Extrapolation
==============================================
Reads h_gen_top_pt histograms from TTbar_run2.root (13 TeV) and
TTbar_run3.root (13.6 TeV), normalises them, computes the ratio
(13.6 TeV / 13 TeV), and fits it with a straight line:

    SF_extrap(pT) = a + b * pT

which corresponds to eq. (4) in the analysis note:
    SF(t) = 0.991 + 0.000075 * pT

The POWHEG top-pT SF from eq. (3) is also shown for reference:
    SF_POWHEG(pT) = 0.103 * exp(-0.0118 * pT) - 0.000134 * pT + 0.973

Requirements:
    pip install uproot numpy scipy matplotlib
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from scipy.optimize import curve_fit
import uproot

# ── file / histogram names ──────────────────────────────────────────────────
FILE_13   = "TTbar_run2.root"   # 13 TeV
FILE_136  = "TTbar_run3.root"   # 13.6 TeV
HIST_NAME = "h_gen_top_pt"

# ── POWHEG SF from eq. (3) ──────────────────────────────────────────────────
def sf_powheg(pt):
    return 0.103 * np.exp(-0.0118 * pt) - 0.000134 * pt + 0.973

# ── linear model for the ratio fit ─────────────────────────────────────────
def linear(pt, a, b):
    return a + b * pt

# ── helpers ─────────────────────────────────────────────────────────────────
def load_hist(filename, histname):
    """Return (bin_centers, values, errors) with the histogram normalised to unit area."""
    with uproot.open(filename) as f:
        h = f[histname]
        values, edges = h.to_numpy()
        errors        = np.sqrt(h.variances())   # stat uncertainties
    centers   = 0.5 * (edges[:-1] + edges[1:])
    bin_widths = edges[1:] - edges[:-1]

    # normalise to unit area
    area    = np.sum(values * bin_widths)
    norm_v  = values  / area
    norm_e  = errors  / area
    return centers, norm_v, norm_e

def ratio_error(a, da, b, db):
    """Gaussian error propagation for ratio a/b."""
    return np.sqrt((da / b)**2 + (a * db / b**2)**2)

# ── load histograms ─────────────────────────────────────────────────────────
print(f"Loading {HIST_NAME} from {FILE_13}  (13 TeV)   …")
pt_c, n13, e13 = load_hist(FILE_13,  HIST_NAME)

print(f"Loading {HIST_NAME} from {FILE_136} (13.6 TeV) …")
_,    n136, e136 = load_hist(FILE_136, HIST_NAME)

# ── ratio: 13.6 TeV / 13 TeV ────────────────────────────────────────────────
# skip bins where either histogram is zero to avoid division by zero
mask  = (n13 > 0) & (n136 > 0)
pt_r  = pt_c[mask]
ratio = n136[mask] / n13[mask]
r_err = ratio_error(n136[mask], e136[mask], n13[mask], e13[mask])

# ── linear fit ──────────────────────────────────────────────────────────────
p0 = [1.0, 0.0]
popt, pcov = curve_fit(linear, pt_r, ratio, p0=p0, sigma=r_err, absolute_sigma=True)
perr = np.sqrt(np.diag(pcov))

a, b = popt
da, db = perr

print("\n── Fit result ─────────────────────────────────────────────────────────")
print(f"  SF_extrap(pT) = ({a:.4f} ± {da:.4f}) + ({b:.6f} ± {db:.6f}) * pT")
print(f"\n  Compare with eq. (4):  SF(t) = 0.991 + 0.000075 * pT")

# ── chi² / ndf ──────────────────────────────────────────────────────────────
residuals = ratio - linear(pt_r, *popt)
chi2      = np.sum((residuals / r_err)**2)
ndf       = len(pt_r) - len(popt)
print(f"\n  χ²/ndf = {chi2:.2f} / {ndf} = {chi2/ndf:.3f}")

# ── plot ────────────────────────────────────────────────────────────────────
pt_fine = np.linspace(pt_c[0], pt_c[-1], 500)

fig, axes = plt.subplots(2, 1, figsize=(8, 10))
fig.suptitle("Top $p_T$ reweighting – extrapolation 13 → 13.6 TeV", fontsize=13)

# ── upper panel: normalised distributions ───────────────────────────────────
ax1 = axes[0]
ax1.step(pt_c, n13,  where="mid", color="royalblue",  label="13 TeV (Run 2)",   lw=1.8)
ax1.step(pt_c, n136, where="mid", color="tomato",     label="13.6 TeV (Run 3)", lw=1.8)
ax1.fill_between(pt_c, n13  - e13,  n13  + e13,  step="mid",
                 color="royalblue", alpha=0.25)
ax1.fill_between(pt_c, n136 - e136, n136 + e136, step="mid",
                 color="tomato",    alpha=0.25)
ax1.set_ylabel("Normalised events / bin", fontsize=11)
ax1.set_xlabel("Generator-level top $p_T$ [GeV]", fontsize=11)
ax1.legend(fontsize=11)
ax1.set_xlim(pt_c[0], pt_c[-1])
ax1.yaxis.set_minor_locator(ticker.AutoMinorLocator())
ax1.xaxis.set_minor_locator(ticker.AutoMinorLocator())
ax1.tick_params(which="both", direction="in", top=True, right=True)
ax1.text(0.97, 0.97,
         r"$\bf{CMS}$ Simulation",
         ha="right", va="top", transform=ax1.transAxes, fontsize=11)

# ── lower panel: ratio + fit ─────────────────────────────────────────────────
ax2 = axes[1]
ax2.errorbar(pt_r, ratio, yerr=r_err,
             fmt="o", color="black", ms=4, lw=1.2,
             label="Ratio 13.6 / 13 TeV", zorder=3)
ax2.plot(pt_fine, linear(pt_fine, *popt), color="crimson", lw=2,
         label=f"Linear fit:  {a:.3f} + {b:.5f}·$p_T$\n"
               r"($\chi^2$/ndf = " + f"{chi2:.1f}/{ndf})")
ax2.axhline(1, color="grey", ls="--", lw=1)

# shade ±1σ band from the fit
cov = pcov
y_band = np.array([np.sqrt(
    np.array([1, p]).T @ cov @ np.array([1, p])
) for p in pt_fine])
ax2.fill_between(pt_fine,
                 linear(pt_fine, *popt) - y_band,
                 linear(pt_fine, *popt) + y_band,
                 color="crimson", alpha=0.15, label="Fit ±1σ")

ax2.set_ylabel("Ratio (13.6 / 13 TeV)", fontsize=11)
ax2.set_xlabel("Generator-level top $p_T$ [GeV]", fontsize=11)
ax2.legend(fontsize=10)
ax2.set_xlim(pt_c[0], pt_c[-1])
ax2.set_ylim(0.85, 1.15)
ax2.axhline(1, color="grey", ls="--", lw=1)
ax2.yaxis.set_minor_locator(ticker.AutoMinorLocator())
ax2.xaxis.set_minor_locator(ticker.AutoMinorLocator())
ax2.tick_params(which="both", direction="in", top=True, right=True)

plt.tight_layout()
out_fig = "top_pt_sf_extrapolation.pdf"
plt.savefig(out_fig, bbox_inches="tight")
print(f"\nFigure saved to  {out_fig}")
plt.show()

# ── convenience: print the combined SF at a few pT values ───────────────────
print("\n── Combined SF = SF_POWHEG × SF_extrap at selected pT values ──────────")
print(f"{'pT [GeV]':>10}  {'SF_POWHEG':>12}  {'SF_extrap':>12}  {'SF_combined':>14}")
for pt_val in [0, 50, 100, 150, 200, 300, 400, 500]:
    sf_p = sf_powheg(pt_val)
    sf_e = linear(pt_val, *popt)
    print(f"{pt_val:>10}  {sf_p:>12.4f}  {sf_e:>12.4f}  {sf_p*sf_e:>14.4f}")
