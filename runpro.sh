#time ./processnanoaod.py --allinone -Y 18 roottestfilesUL18/ testBaseANA_UL18.root >testBaseUL.out
time ./processnanoaod.py --allinone  -Y 2018 --runtype UL --nrootfiles 1 --isDATA 0 roottestfilesUL18/ testBaseANA_UL18.root >testBaseUL.out
#time ./processnanoaod.py  --nrootfiles 4 --split 4 --isDATA 0  --runtype UL -Y 2018 roottestfilesUL18/ testBaseANA_out >testBaseUL.out #(if not allinone)

	#info:
	#structure:
	#time :linux command :
			#Real: This is the time taken from when the call was given till the point the call is completed. This is the time that has passed when measured in real-time.
			#User: time is how long your program was running on the CPU
			#Sys:  time was how long your program was waiting for the operating system to perform tasks for it.

	# ./processnanoaod.py [option] [inputdir] [outputdir]

	#NOTE THAT! if you want to use --alinone option use output.root file instead of a directory (outputdir)

	#--allinone (-A) 	: will let you get a single output root file for all the files in the inputDir.
	#--year (-Y) 		: year option (2017, 2018)
	#--runtype (-R)		: To choose Ultra Legacy (UL)  or ReReco run
	#--nrootfiles (-N)	: option to select max root files
	#--isDATA 			: option select Data or MC
	#						==> default is -1 (without this option or by typing -1 ): selection will be done authomatically by checking the genweight branch
	#						==> 0 : MC
	#						==> 1 : DATA

	#roottestfilesUL18 	: input root files directory [inputdir]
	#testBaseANA_out	: output root file directory [outputdir]
	#testBaseANA_UL18.root : output root file name 	[output.root] = for --allinone option
