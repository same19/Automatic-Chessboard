import serial
import time
from stockfish import Stockfish

# stockfish = Stockfish(path="/Users/samengel/Documents/ASR/'Chess Board'/stockfish-3.28.0/stockfish")
stockfish = Stockfish()

#the string needs to match the port of the arduino (e.g. "/dev/cu.usbmodem101")
arduino = serial.Serial("/dev/cu.usbmodem1101", 9600)
time.sleep(2)

stockfish.set_position([])

while True:
	print(stockfish.get_board_visual())
	print("Eval: "+str(float(stockfish.get_evaluation()['value'])/100))
	whiteMove = str(input("White's turn: "))
	if (stockfish.is_move_correct(whiteMove)):
		if stockfish.will_move_be_a_capture(whiteMove) == Stockfish.Capture.DIRECT_CAPTURE: #capture!
			print("Capture!")
			arduino.write(str.encode(whiteMove[2:] + "---")) #move captured piece out of the way
			tdata = arduino.read()
		time.sleep(1)
		# else if (stockfish.will_move_be_a_capture(whiteMove) == Stockfish.Capture.EN_PASSANT):
		# 	print("En Passant! Please manually remove the captured pawn.")
		arduino.write(str.encode(whiteMove + stockfish.get_what_is_on_square(whiteMove[:2]).value))
		tdata = arduino.read()
		print("DATA: ")
		print(tdata)
		stockfish.make_moves_from_current_position([whiteMove])
	else:
		print("Invalid move.")
		continue
	blackMove = stockfish.get_best_move()
	print("Black's turn: " + blackMove)
	time.sleep(1)
	if stockfish.will_move_be_a_capture(blackMove) == Stockfish.Capture.DIRECT_CAPTURE: #capture!
		print("Capture!")
		arduino.write(str.encode(blackMove[2:] + "---")) #move captured piece out of the way
		tdata = arduino.read()
	# else if (stockfish.will_move_be_a_capture(whiteMove) == Stockfish.Capture.EN_PASSANT):
	# 	print("En Passant! Please manually remove the captured pawn.")
	time.sleep(1)
	arduino.write(str.encode(blackMove + stockfish.get_what_is_on_square(blackMove[:2]).value))
	tdata = arduino.read()
	stockfish.make_moves_from_current_position([blackMove])