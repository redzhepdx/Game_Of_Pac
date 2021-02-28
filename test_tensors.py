import torch
import numpy as np

def main():
	x = np.array([
		0.01 * 3.6056,
 		0.01 * 3.6056,
		0.01 *-4.4359,
 		0.01 * 2.7594,
 		0.01 * 2.7795,
	 	0.01 * 2.7795,
		0.01 * 2.7594,
 		0.01 * 3.8333,
		0.01 * -4.4531,
		 0.01 *2.7594,
		])
	y = np.array([-5.0057,
		-5.0057,
		-4.9985,
		-4.9985,
		-4.9985,
		-4.9985,
		-4.9985,
		-4.9986,
		-4.9985,
		-4.9985
		])

	tx = torch.from_numpy(x)
	ty = torch.from_numpy(y)
	mse_val = torch.nn.functional.mse_loss(tx, ty) 
	print(mse_val.item())

if __name__ == "__main__":
	main()
