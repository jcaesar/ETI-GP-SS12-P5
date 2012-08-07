protected void paintComponent(Graphics g) {
	Graphics2D g2 = (Graphics2D) g;
		
	AffineTransform tx = new AffineTransform();
	tx.translate(dx * scale, dy * scale);
	tx.scale(scale, scale);		
	g2.setTransform(tx);

	//2-fache for-Schleife:
	g2.setColor(colors[i][j]);
	g2.fillRect(i, j, 1, 1);
}