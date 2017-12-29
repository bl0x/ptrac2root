#include <TObject.h>

class PtracStep : public TObject
{
	public:
		int fNextEventType;
		int fNumberOfNodes; /* NODE */
		int fSourceNumber;  /* NSR */
		int fBankSource;
		int fZZAAA;         /* NXS */
		int fReactionType;  /* NYTN */
		int fSurfaceNumber; /* NSF */
		int fAngle;         /* in degrees */
		int fTerminationType; /* NTER */
		int fBranchNumber;
		int fParticleType;  /* IPT */
		int fCellNumber;    /* NCL */
		int fMaterialNumber; /* MAT */
		int fNumberOfCollisions; /* NCP */

		double fX;
		double fY;
		double fZ;
		double fU;
		double fV;
		double fW;
		double fEnergy;
		double fWeight;
		double fTime;

	ClassDef(PtracStep, 1)
};

