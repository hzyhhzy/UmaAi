#python -u train.py --gpu 0 --modeltype res --modelparam 10 512 --savename auto --lrscale 1.0  --wdscale 1.0 --rollbackthreshold 0.2 --batchsize 1024
#python -u train.py --gpu 0 --modeltype ems2 --modelparam 1 384 3 384 384 --savename auto --lrscale 1.0  --wdscale 1.0 --rollbackthreshold 0.2 --batchsize 1024
#CUDA_VISIBLE_DEVICES="2" 
python -u train.py --gpu 0 --modeltype ems --modelparam 1 256 2 256 256 --savename auto --lrscale 0.5  --wdscale 2.0 --rollbackthreshold 0.2 --batchsize 8192 --datathread 16